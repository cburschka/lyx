/**
 * \file bufferlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "bufferlist.h"

#include "author.h"
#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "gettext.h"
#include "lastfiles.h"
#include "lyx_cb.h"
#include "lyx_main.h"
#include "output_latex.h"
#include "paragraph.h"
#include "ParagraphList_fwd.h"

#include "frontends/Alert.h"

#include "support/filetools.h"
#include "support/os.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

using lyx::support::AddName;
using lyx::support::bformat;
using lyx::support::GetEnvPath;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::removeAutosaveFile;
using lyx::support::prefixIs;

using boost::bind;

using std::auto_ptr;
using std::endl;
using std::equal_to;
using std::find;
using std::find_if;
using std::for_each;
using std::string;
using std::vector;
using std::back_inserter;
using std::transform;

namespace os = lyx::support::os;

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
		"Do you want to save the document or discard the changes?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));

	if (ret == 0) {
		// FIXME: WriteAs can be asynch !
		// but not right now...maybe we should remove that

		bool succeeded;

		if (buf->isUnnamed())
			succeeded = WriteAs(buf);
		else
			succeeded = MenuWrite(buf);

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
	BOOST_ASSERT(buf);
	BufferStorage::iterator it = find(bstore.begin(), bstore.end(), buf);
	if (it != bstore.end()) {
		Buffer * tmp = (*it);
		bstore.erase(it);
		delete tmp;
	}
}


Buffer * BufferList::newBuffer(string const & s, bool ronly)
{
	auto_ptr<Buffer> tmpbuf(new Buffer(s, ronly));
	tmpbuf->params().useClassDefaults();
	lyxerr[Debug::INFO] << "Assigning to buffer "
			    << bstore.size() << endl;
	bstore.push_back(tmpbuf.get());
	return tmpbuf.release();
}


void BufferList::closeAll()
{
	while (!bstore.empty()) {
		close(bstore.front(), false);
	}
}


bool BufferList::close(Buffer * buf, bool ask)
{
	BOOST_ASSERT(buf);

	// FIXME: is the quitting check still necessary ?
	if (!ask || buf->isClean() || quitting || buf->paragraphs().empty()) {
		release(buf);
		return true;
	}

	string fname;
	if (buf->isUnnamed())
		fname = OnlyFilename(buf->fileName());
	else
		fname = MakeDisplayPath(buf->fileName(), 30);

	string text = bformat(_("The document %1$s has unsaved changes.\n\n"
		"Do you want to save the document or discard the changes?"), fname);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));

	if (ret == 0) {
		if (buf->isUnnamed()) {
			if (!WriteAs(buf))
				return false;
		} else if (buf->save()) {
			LyX::ref().lastfiles().newFile(buf->fileName());
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
	transform(bstore.begin(), bstore.end(),
		  back_inserter(nvec),
		  boost::bind(&Buffer::fileName, _1));
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


Buffer * BufferList::next(Buffer const * buf) const
{
	if (bstore.empty())
		return 0;
	BufferStorage::const_iterator it = find(bstore.begin(),
						bstore.end(), buf);
	BOOST_ASSERT(it != bstore.end());
	++it;
	if (it == bstore.end())
		return bstore.front();
	else
		return *it;
}


Buffer * BufferList::previous(Buffer const * buf) const
{
	if (bstore.empty())
		return 0;
	BufferStorage::const_iterator it = find(bstore.begin(),
						bstore.end(), buf);
	BOOST_ASSERT(it != bstore.end());
	if (it == bstore.begin())
		return bstore.back();
	else
		return *(it - 1);
}


void BufferList::updateIncludedTeXfiles(string const & mastertmpdir,
					OutputParams const & runparams)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		if (!(*it)->isDepClean(mastertmpdir)) {
			string writefile = mastertmpdir;
			writefile += '/';
			writefile += (*it)->getLatexName();
			(*it)->makeLaTeXFile(writefile, mastertmpdir,
					     runparams, false);
			(*it)->markDepClean(mastertmpdir);
		}
	}
}


void BufferList::emergencyWriteAll()
{
	for_each(bstore.begin(), bstore.end(),
		 bind(&BufferList::emergencyWrite, this, _1));
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
	string s = AddName(os::homepath(), buf->fileName());
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


bool BufferList::exists(string const & s) const
{
	return find_if(bstore.begin(), bstore.end(),
		       bind(equal_to<string>(),
			    bind(&Buffer::fileName, _1),
			    s))
		!= bstore.end();
}


bool BufferList::isLoaded(Buffer const * b) const
{
	BOOST_ASSERT(b);
	BufferStorage::const_iterator cit =
		find(bstore.begin(), bstore.end(), b);
	return cit != bstore.end();
}


Buffer * BufferList::getBuffer(string const & s)
{
	BufferStorage::iterator it =
		find_if(bstore.begin(), bstore.end(),
			bind(equal_to<string>(),
			     bind(&Buffer::fileName, _1),
			     s));

	return it != bstore.end() ? (*it) : 0;
}


Buffer * BufferList::getBufferFromTmp(string const & s)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it < end; ++it)
		if (prefixIs(s, (*it)->temppath()))
			return *it;
	return 0;
}


void BufferList::setCurrentAuthor(string const & name, string const & email)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		(*it)->params().authors().record(0, Author(name, email));
	}
}
