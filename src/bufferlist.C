/**
 * \file bufferlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "bufferlist.h"
#include "lyx_main.h"
#include "lastfiles.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "bufferview_funcs.h"
#include "BufferView.h"
#include "gettext.h"
#include "frontends/LyXView.h"
#include "vc-backend.h"
#include "TextCache.h"
#include "lyxlex.h"

#include "frontends/Alert.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lyxmanip.h"
#include "support/lyxfunctional.h"
#include "support/LAssert.h"

#include <boost/bind.hpp>

#include <cassert>
#include <algorithm>
#include <functional>


using std::vector;
using std::find;
using std::endl;
using std::find_if;
using std::for_each;
using std::mem_fun;

extern BufferView * current_view;


BufferList::BufferList()
{}


bool BufferList::empty() const
{
	return bstore.empty();
}


bool BufferList::quitWriteBuffer(Buffer * buf)
{
	string file;
	if (buf->isUnnamed())
		file = OnlyFilename(buf->fileName());
	else
		file = MakeDisplayPath(buf->fileName(), 30);

	string text = bformat(_("The document %1$s has unsaved changes.\n\n"
		"Do you want to save the document?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save Changes"), _("&Discard Changes"), _("&Cancel"));

	if (ret == 0) {
		// FIXME: WriteAs can be asynch !
		// but not right now...maybe we should remove that

		bool succeeded;

		if (buf->isUnnamed())
			succeeded = WriteAs(current_view, buf);
		else
			succeeded = MenuWrite(current_view, buf);

		if (!succeeded)
			return false;
	} else if (ret == 1) {
		// if we crash after this we could
		// have no autosave file but I guess
		// this is really inprobable (Jug)
		if (buf->isUnnamed())
			removeAutosaveFile(buf->fileName());

	} else {
		return false;
	}

	return true;
}


bool BufferList::quitWriteAll()
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		if ((*it)->isClean())
			continue;

		if (!quitWriteBuffer(*it))
			return false;
	}

	return true;
}


void BufferList::release(Buffer * buf)
{
	lyx::Assert(buf);
	BufferStorage::iterator it = find(bstore.begin(), bstore.end(), buf);
	if (it != bstore.end()) {
		// Make sure that we don't store a LyXText in
		// the textcache that points to the buffer
		// we just deleted.
		Buffer * tmp = (*it);
		bstore.erase(it);
		textcache.removeAllWithBuffer(tmp);
		delete tmp;
	}
}


Buffer * BufferList::newBuffer(string const & s, bool ronly)
{
	Buffer * tmpbuf = new Buffer(s, ronly);
	tmpbuf->params.useClassDefaults();
	lyxerr[Debug::INFO] << "Assigning to buffer "
			    << bstore.size() << endl;
	bstore.push_back(tmpbuf);
	return tmpbuf;
}


void BufferList::closeAll()
{
	// Since we are closing we can just as well delete all
	// in the textcache this will also speed the closing/quiting up a bit.
	textcache.clear();

	while (!bstore.empty()) {
		close(bstore.front(), false);
	}
}


bool BufferList::close(Buffer * buf, bool ask)
{
	lyx::Assert(buf);

	// FIXME: is the quitting check still necessary ?
	if (!ask || buf->isClean() || quitting || buf->paragraphs.empty()) {
		release(buf);
		return true;
	}

	string fname;
	if (buf->isUnnamed())
		fname = OnlyFilename(buf->fileName());
	else
		fname = MakeDisplayPath(buf->fileName(), 30);

	string text = bformat(_("The document %1$s has unsaved changes.\n\n"
		"Do you want to save the document?"), fname);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save Changes"), _("&Discard Changes"), _("&Cancel"));

	if (ret == 0) {
		if (buf->isUnnamed()) {
			if (!WriteAs(current_view, buf))
				return false;
		} else if (buf->save()) {
			lastfiles->newFile(buf->fileName());
		} else {
			return false;
		}
	} else if (ret == 2) {
		return false;
	}

	if (buf->isUnnamed()) {
		removeAutosaveFile(buf->fileName());
	}

	release(buf);
	return true;
}


vector<string> const BufferList::getFileNames() const
{
	vector<string> nvec;
	std::copy(bstore.begin(), bstore.end(),
		  lyx::back_inserter_fun(nvec, &Buffer::fileName));
	return nvec;
}


Buffer * BufferList::first()
{
	if (bstore.empty())
		return 0;
	return bstore.front();
}


Buffer * BufferList::getBuffer(unsigned int choice)
{
	if (choice >= bstore.size())
		return 0;
	return bstore[choice];
}


void BufferList::updateIncludedTeXfiles(string const & mastertmpdir)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		if (!(*it)->isDepClean(mastertmpdir)) {
			string writefile = mastertmpdir;
			writefile += '/';
			writefile += (*it)->getLatexName();
			(*it)->makeLaTeXFile(writefile, mastertmpdir,
					     false, true);
			(*it)->markDepClean(mastertmpdir);
		}
	}
}


void BufferList::emergencyWriteAll()
{
	for_each(bstore.begin(), bstore.end(),
		 boost::bind(&BufferList::emergencyWrite, this, _1));
}


void BufferList::emergencyWrite(Buffer * buf)
{
	// assert(buf) // this is not good since C assert takes an int
		       // and a pointer is a long (JMarc)
	assert(buf != 0); // use c assert to avoid a loop


	// No need to save if the buffer has not changed.
	if (buf->isClean())
		return;

	string const doc = buf->isUnnamed()
		? OnlyFilename(buf->fileName()) : buf->fileName();

	lyxerr << bformat(_("LyX: Attempting to save document %1$s"), doc) << endl;

	// We try to save three places:
	// 1) Same place as document. Unless it is an unnamed doc.
	if (!buf->isUnnamed()) {
		string s = buf->fileName();
		s += ".emergency";
		lyxerr << "  " << s << endl;
		if (buf->writeFile(s)) {
			buf->markClean();
			lyxerr << _("  Save seems successful. Phew.") << endl;
			return;
		} else {
			lyxerr << _("  Save failed! Trying...") << endl;
		}
	}

	// 2) In HOME directory.
	string s = AddName(GetEnvPath("HOME"), buf->fileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (buf->writeFile(s)) {
		buf->markClean();
		lyxerr << _("  Save seems successful. Phew.") << endl;
		return;
	}

	lyxerr << _("  Save failed! Trying...") << endl;

	// 3) In "/tmp" directory.
	// MakeAbsPath to prepend the current
	// drive letter on OS/2
	s = AddName(MakeAbsPath("/tmp/"), buf->fileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (buf->writeFile(s)) {
		buf->markClean();
		lyxerr << _("  Save seems successful. Phew.") << endl;
		return;
	}
	lyxerr << _("  Save failed! Bummer. Document is lost.") << endl;
}



Buffer * BufferList::readFile(string const & s, bool ronly)
{
	string ts(s);
	string e = OnlyPath(s);
	string a = e;
	// File information about normal file
	FileInfo fileInfo2(s);

	if (!fileInfo2.exist()) {
		string const file = MakeDisplayPath(s, 50);
		string text = bformat(_("The specified document\n%1$s"
			"\ncould not be read."),	file);
		Alert::error(_("Could not read document"), text);
		return 0;
	}

	Buffer * b = newBuffer(s, ronly);

	// Check if emergency save file exists and is newer.
	e += OnlyFilename(s) + ".emergency";
	FileInfo fileInfoE(e);

	bool use_emergency = false;

	if (fileInfoE.exist() && fileInfo2.exist()) {
		if (fileInfoE.getModificationTime()
		    > fileInfo2.getModificationTime()) {
			string const file = MakeDisplayPath(s, 20);
			string text = bformat(_("An emergency save of the document %1$s exists.\n"
				"\nRecover emergency save?"), file);
			int const ret = Alert::prompt(_("Load emergency save?"),
				text, 0, 1, _("&Recover"), _("&Load Original"));

			if (ret == 0) {
				ts = e;
				// the file is not saved if we load the
				// emergency file.
				b->markDirty();
				use_emergency = true;
			}
		}
	}

	if (!use_emergency) {
		// Now check if autosave file is newer.
		a += '#';
		a += OnlyFilename(s);
		a += '#';
		FileInfo fileInfoA(a);
		if (fileInfoA.exist() && fileInfo2.exist()) {
			if (fileInfoA.getModificationTime()
			    > fileInfo2.getModificationTime()) {
				string const file = MakeDisplayPath(s, 20);
				string text = bformat(_("The backup of the document %1$s is newer.\n\n"	
					"Load the backup instead?"), file);
				int const ret = Alert::prompt(_("Load backup?"),
					text, 0, 1, _("&Load backup"), _("Load &original"));

				if (ret == 0) {
					ts = a;
					// the file is not saved if we load the
					// autosave file.
					b->markDirty();
				} else {
					// Here, we should delete the autosave
					lyx::unlink(a);
				}
			}
		}
	}
	// not sure if this is the correct place to begin LyXLex
	LyXLex lex(0, 0);
	lex.setFile(ts);
	if (b->readFile(lex, ts))
		return b;
	else {
		release(b);
		return 0;
	}
}


bool BufferList::exists(string const & s) const
{
	return find_if(bstore.begin(), bstore.end(),
		       lyx::compare_memfun(&Buffer::fileName, s))
		!= bstore.end();
}


bool BufferList::isLoaded(Buffer const * b) const
{
	lyx::Assert(b);

	BufferStorage::const_iterator cit =
		find(bstore.begin(), bstore.end(), b);
	return cit != bstore.end();
}


Buffer * BufferList::getBuffer(string const & s)
{
	BufferStorage::iterator it =
		find_if(bstore.begin(), bstore.end(),
			lyx::compare_memfun(&Buffer::fileName, s));
	return it != bstore.end() ? (*it) : 0;
}


Buffer * BufferList::newFile(string const & name, string tname, bool isNamed)
{
	// get a free buffer
	Buffer * b = newBuffer(name);

	// use defaults.lyx as a default template if it exists.
	if (tname.empty()) {
		tname = LibFileSearch("templates", "defaults.lyx");
	}
	if (!tname.empty()) {
		bool templateok = false;
		LyXLex lex(0, 0);
		lex.setFile(tname);
		if (lex.isOK()) {
			if (b->readFile(lex, tname)) {
				templateok = true;
			}
		}
		if (!templateok) {
			string const file = MakeDisplayPath(tname, 50);
			string text  = bformat(_("The specified document template\n%1$s\n"
				"could not be read."), file);
			Alert::error(_("Could not read template"), text);
			// no template, start with empty buffer
			b->paragraphs.push_back(new Paragraph);
			b->paragraphs.begin()->layout(b->params.getLyXTextClass().defaultLayout());
		}
	} else {  // start with empty buffer
		b->paragraphs.push_back(new Paragraph);
		b->paragraphs.begin()->layout(b->params.getLyXTextClass().defaultLayout());
	}

	if (!isNamed) {
		b->setUnnamed();
		b->setFileName(name);
	}

	b->setReadonly(false);
	b->updateDocLang(b->params.language);

	return b;
}


Buffer * BufferList::loadLyXFile(string const & filename, bool tolastfiles)
{
	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	string s = FileSearch(string(), filename, "lyx");
	if (s.empty()) {
		s = filename;
	}

	// file already open?
	if (exists(s)) {
		string const file = MakeDisplayPath(s, 20);
		string text = bformat(_("The document %1$s is already loaded.\n\n"
			"Do you want to revert to the saved version?"), file);
		int const ret = Alert::prompt(_("Revert to saved document?"),
			text, 0, 1,  _("&Revert"), _("&Switch to document"));

		if (ret == 0) {
			// FIXME: should be LFUN_REVERT
			if (!close(getBuffer(s), false)) {
				return 0;
			}
			// Fall through to new load. (Asger)
		} else {
			// Here, we pretend that we just loaded the
			// open document
			return getBuffer(s);
		}
	}

	Buffer * b = 0;
	bool ro = false;
	switch (IsFileWriteable(s)) {
	case 0:
		ro = true;
		// Fall through
	case 1:
		b = readFile(s, ro);
		if (b) {
			b->lyxvc.file_found_hook(s);
		}
		break; //fine- it's r/w
	case -1: {
		string const file = MakeDisplayPath(s, 20);
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			string text = bformat(_("Do you want to retrieve the document"
				" %1$s from version control?"), file);
			int const ret = Alert::prompt(_("Retrieve from version control?"),
				text, 0, 1, _("&Retrieve"), _("&Cancel"));

			if (ret == 0) {
				// How can we know _how_ to do the checkout?
				// With the current VC support it has to be,
				// a RCS file since CVS do not have special ,v files.
				RCS::retrieve(s);
				return loadLyXFile(filename, tolastfiles);
			}
		}

		string text = bformat(_("The document %1$s does not yet exist.\n\n"
			"Do you want to create a new document?"), file);
		int const ret = Alert::prompt(_("Create new document?"),
			text, 0, 1, _("&Create"), _("Cancel"));

		if (ret == 0)
			b = newFile(s, string(), true);

		break;
		}
	}

	if (b && tolastfiles)
		lastfiles->newFile(b->fileName());

	return b;
}


void BufferList::setCurrentAuthor(string const & name, string const & email)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		(*it)->authors().record(0, Author(name, email));
	}
}
