/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Word Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team. 
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include <algorithm>
#include <functional>

#include "bufferlist.h"
#include "lyx_main.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lyxmanip.h"
#include "support/lyxfunctional.h"
#include "lyx_gui_misc.h"
#include "lastfiles.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "bufferview_funcs.h"
#include "BufferView.h"
#include "gettext.h"
#include "LyXView.h"
#include "vc-backend.h"
#include "TextCache.h"

using std::vector;
using std::find;
using std::endl;
using std::find_if;
using std::for_each;
using std::mem_fun;

extern BufferView * current_view;

//
// Class BufferStorage
//

void BufferStorage::release(Buffer * buf)
{
	lyx::Assert(buf);
	Container::iterator it = find(container.begin(), container.end(), buf);
	if (it != container.end()) {
		// Make sure that we don't store a LyXText in
		// the textcache that points to the buffer
		// we just deleted.
		Buffer * tmp = (*it);
		container.erase(it);
		textcache.removeAllWithBuffer(tmp);
		delete tmp;
	}
}


Buffer * BufferStorage::newBuffer(string const & s, bool ronly)
{
	Buffer * tmpbuf = new Buffer(s, ronly);
	tmpbuf->params.useClassDefaults();
	lyxerr[Debug::INFO] << "Assigning to buffer "
			    << container.size() << endl;
	container.push_back(tmpbuf);
	return tmpbuf;
}


//
// Class BufferList
//

BufferList::BufferList()
	: state_(BufferList::OK)
{}


bool BufferList::empty() const
{
	return bstore.empty();
}


bool BufferList::qwriteAll()
{
        bool askMoreConfirmation = false;
        string unsaved;
	for (BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if (!(*it)->isLyxClean()) {
			string fname;
			if ((*it)->isUnnamed())
				fname = OnlyFilename((*it)->fileName());
			else
				fname = MakeDisplayPath((*it)->fileName(), 50);
			bool reask = true;
			while (reask) {
				switch (AskConfirmation(_("Changes in document:"),
						       fname,
						       _("Save document?"))) {
				case 1: // Yes
					if ((*it)->isUnnamed())
						reask = !WriteAs(current_view, (*it));
					else {
						reask = !MenuWrite(current_view, (*it));
					}
					break;
				case 2: // No
					// if we crash after this we could
					// have no autosave file but I guess
					// this is really inprobable (Jug)
					if ((*it)->isUnnamed()) {
						removeAutosaveFile((*it)->fileName());
					}
					askMoreConfirmation = true;
					unsaved += MakeDisplayPath(fname, 50);
					unsaved += "\n";
					reask = false;
					break;
				case 3: // Cancel
					return false;
				}
			}
		}
	}
        if (askMoreConfirmation &&
            lyxrc.exit_confirmation &&
            !AskQuestion(_("Some documents were not saved:"),
                         unsaved, _("Exit anyway?"))) {
                return false;
        }

        return true;
}


void BufferList::closeAll()
{
	state_ = BufferList::CLOSING;
	// Since we are closing we can just as well delete all
	// in the textcache this will also speed the closing/quiting up a bit.
	textcache.clear();
	
	while (!bstore.empty()) {
		close(bstore.front());
	}
	state_ = BufferList::OK;
}


bool BufferList::close(Buffer * buf)
{
	lyx::Assert(buf);
	
	// CHECK
	// Trace back why we need to use buf->getUser here.
	// Perhaps slight rewrite is in order? (Lgb)
	
        if (buf->getUser()) buf->getUser()->insetUnlock();
	if (buf->paragraph && !buf->isLyxClean() && !quitting) {
		if (buf->getUser())
			ProhibitInput(buf->getUser());
		string fname;
		if (buf->isUnnamed())
			fname = OnlyFilename(buf->fileName());
		else
			fname = MakeDisplayPath(buf->fileName(), 50);
		bool reask = true;
		while (reask) {
			switch (AskConfirmation(_("Changes in document:"),
					       fname,
					       _("Save document?"))){
			case 1: // Yes
				if (buf->isUnnamed())
					reask = !WriteAs(current_view, buf);
				else if (buf->save()) {
					lastfiles->newFile(buf->fileName());
					reask = false;
				} else {
					if (buf->getUser())
						AllowInput(buf->getUser());
					return false;
				}
				break;
			case 2:
				if (buf->isUnnamed()) {
					removeAutosaveFile(buf->fileName());
				}
				reask = false;
				break;
			case 3: // Cancel
				if (buf->getUser())
					AllowInput(buf->getUser());
				return false;
			}
		}
		if (buf->getUser())
			AllowInput(buf->getUser());
	}

	bstore.release(buf);
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
	if (bstore.empty()) return 0;
	return bstore.front();
}


Buffer * BufferList::getBuffer(unsigned int choice)
{
	if (choice >= bstore.size()) return 0;
	return bstore[choice];
}


int BufferList::unlockInset(UpdatableInset * inset)
{
	lyx::Assert(inset);
	
	//if (!inset) return 1;
	for (BufferStorage::iterator it = bstore.begin();
	     it != bstore.end(); ++it) {
		if ((*it)->getUser()
		    && (*it)->getUser()->theLockingInset() == inset) {
			(*it)->getUser()->insetUnlock();
			return 0;
		}
	}
	return 1;
}


void BufferList::updateIncludedTeXfiles(string const & mastertmpdir)
{
	for (BufferStorage::iterator it = bstore.begin();
	     it != bstore.end(); ++it) {
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
		 lyx::class_fun(*this, &BufferList::emergencyWrite));
}


void BufferList::emergencyWrite(Buffer * buf) 
{
	// assert(buf) // this is not good since C assert takes an int
	               // and a pointer is a long (JMarc)
	assert(buf != 0); // use c assert to avoid a loop

	
	// No need to save if the buffer has not changed.
	if (buf->isLyxClean()) return;
	
	lyxerr << fmt(_("lyx: Attempting to save document %s as..."),
		      buf->isUnnamed() ? OnlyFilename(buf->fileName()).c_str()
		      : buf->fileName().c_str()) << endl;
	
	// We try to save three places:

	// 1) Same place as document. Unless it is an unnamed doc.
	if (!buf->isUnnamed()) {
		string s = buf->fileName();
		s += ".emergency";
		lyxerr << "  " << s << endl;
		if (buf->writeFile(s, true)) {
			buf->markLyxClean();
			lyxerr << _("  Save seems successful. Phew.") << endl;
			return;
		} else {
			lyxerr << _("  Save failed! Trying...") << endl;
		}
	}
	
	// 2) In HOME directory.
	string s = AddName(GetEnvPath("HOME"), buf->fileName());
	s += ".emergency";
	lyxerr << " " << s << endl;
	if (buf->writeFile(s, true)) {
		buf->markLyxClean();
		lyxerr << _("  Save seems successful. Phew.") << endl;
		return;
	}
	
	lyxerr << _("  Save failed! Trying...") << endl;
	
	// 3) In "/tmp" directory.
	// MakeAbsPath to prepend the current
	// drive letter on OS/2
	s = AddName(MakeAbsPath("/tmp/"), buf->fileName());
	s += ".emergency";
	lyxerr << " " << s << endl;
	if (buf->writeFile(s, true)) {
		buf->markLyxClean();
		lyxerr << _("  Save seems successful. Phew.") << endl;
		return;
	}
	lyxerr << _("  Save failed! Bummer. Document is lost.") << endl;
}



Buffer * BufferList::readFile(string const & s, bool ronly)
{
	Buffer * b = bstore.newBuffer(s, ronly);

	string ts = s;
	string e = OnlyPath(s);
	string a = e;
	// File information about normal file
	FileInfo fileInfo2(s);

	// Check if emergency save file exists and is newer.
	e += OnlyFilename(s) + ".emergency";
	FileInfo fileInfoE(e);

	bool use_emergency = false;

	if (fileInfoE.exist() && fileInfo2.exist()) {
		if (fileInfoE.getModificationTime()
		    > fileInfo2.getModificationTime()) {
			if (AskQuestion(_("An emergency save of this document exists!"),
					MakeDisplayPath(s, 50),
					_("Try to load that instead?"))) {
				ts = e;
				// the file is not saved if we load the
				// emergency file.
				b->markDirty();
				use_emergency = true;
			} else {
				// Here, we should delete the emergency save
				lyx::unlink(e);
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
				if (AskQuestion(_("Autosave file is newer."),
						MakeDisplayPath(s, 50),
						_("Load that one instead?"))) {
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
	if (b->readFile(lex))
		return b;
	else {
		bstore.release(b);
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
	Buffer * b = bstore.newBuffer(name);

	// use defaults.lyx as a default template if it exists.
	if (tname.empty()) {
		tname = LibFileSearch("templates", "defaults.lyx");
	}
	if (!tname.empty()) {
		bool templateok = false;
		LyXLex lex(0, 0);
		lex.setFile(tname);
		if (lex.IsOK()) {
			if (b->readFile(lex)) {
				templateok = true;
			}
		}
		if (!templateok) {
			WriteAlert(_("Error!"), _("Unable to open template"), 
				   MakeDisplayPath(tname));
			// no template, start with empty buffer
			b->paragraph = new Paragraph;
		}
	} else {  // start with empty buffer
		b->paragraph = new Paragraph;
	}

	if (!lyxrc.new_ask_filename && !isNamed) {
		b->setUnnamed();
		b->setFileName(name);
	}

	b->setReadonly(false);
	
	return b;
}


Buffer * BufferList::loadLyXFile(string const & filename, bool tolastfiles)
{
	// make sure our path is absolute
	string const s = MakeAbsPath(filename);

	// file already open?
	if (exists(s)) {
		if (AskQuestion(_("Document is already open:"), 
				MakeDisplayPath(s, 50),
				_("Do you want to reload that document?"))) {
			// Reload is accomplished by closing and then loading
			if (!close(getBuffer(s))) {
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
	case -1:
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			// Ask if the file should be checked out for
			// viewing/editing, if so: load it.
		        if (AskQuestion(_("Do you want to retrieve file under version control?"))) {
				// How can we know _how_ to do the checkout?
				// With the current VC support it has to be,
				// a RCS file since CVS do not have special ,v files.
				RCS::retrive(s);
				return loadLyXFile(filename, tolastfiles);
			}
		}
		if (AskQuestion(_("Cannot open specified file:"), 
				MakeDisplayPath(s, 50),
				_("Create new document with this name?")))
			{
				// Find a free buffer
				b = newFile(s, string(), true);
			}
		break;
	}

	if (b && tolastfiles)
		lastfiles->newFile(b->fileName());

	return b;
}
