/**
 * \file BufferList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferList.h"

#include "Author.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h"
#include "gettext.h"
#include "Session.h"
#include "callback.h"
#include "LyX.h"
#include "output_latex.h"
#include "ParagraphList.h"

#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/Package.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

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


namespace lyx {

using support::addName;
using support::bformat;
using support::FileName;
using support::makeDisplayPath;
using support::onlyFilename;
using support::removeAutosaveFile;
using support::package;
using support::prefixIs;

namespace Alert = lyx::frontend::Alert;


BufferList::BufferList()
{}


bool BufferList::empty() const
{
	return bstore.empty();
}


BufferList::iterator BufferList::begin()
{
	return bstore.begin();
}


BufferList::const_iterator BufferList::begin() const
{
	return bstore.begin();
}


BufferList::iterator BufferList::end()
{
	return bstore.end();
}


BufferList::const_iterator BufferList::end() const
{
	return bstore.end();
}


bool BufferList::quitWriteBuffer(Buffer * buf)
{
	BOOST_ASSERT(buf);

	docstring file;
	if (buf->isUnnamed())
		file = from_utf8(onlyFilename(buf->fileName()));
	else
		file = makeDisplayPath(buf->fileName(), 30);

	docstring const text =
		bformat(_("The document %1$s has unsaved changes.\n\n"
				       "Do you want to save the document or discard the changes?"),
					   file);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));

	if (ret == 0) {
		// FIXME: WriteAs can be asynch !
		// but not right now...maybe we should remove that

		bool succeeded;

		if (buf->isUnnamed())
			succeeded = writeAs(buf);
		else
			succeeded = menuWrite(buf);

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
	// now, all buffers have been written sucessfully
	// save file names to .lyx/session
	it = bstore.begin();
	for (; it != end; ++it) {
		// if master/slave are both open, do not save slave since it
		// will be automatically loaded when the master is loaded
		if ((*it)->getMasterBuffer() == (*it))
			LyX::ref().session().lastOpened().add(FileName((*it)->fileName()));
	}

	return true;
}


void BufferList::release(Buffer * buf)
{
	BOOST_ASSERT(buf);
	BufferStorage::iterator const it =
		find(bstore.begin(), bstore.end(), buf);
	if (it != bstore.end()) {
		Buffer * tmp = (*it);
		BOOST_ASSERT(tmp);
		bstore.erase(it);
		delete tmp;
	}
}


Buffer * BufferList::newBuffer(string const & s, bool const ronly)
{
	auto_ptr<Buffer> tmpbuf(new Buffer(s, ronly));
	tmpbuf->params().useClassDefaults();
	LYXERR(Debug::INFO) << "Assigning to buffer "
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


bool BufferList::close(Buffer * buf, bool const ask)
{
	BOOST_ASSERT(buf);

	if (!ask || buf->isClean() || buf->paragraphs().empty()) {
		release(buf);
		return true;
	}

	docstring fname;
	if (buf->isUnnamed())
		fname = from_utf8(onlyFilename(buf->fileName()));
	else
		fname = makeDisplayPath(buf->fileName(), 30);

	docstring const text =
		bformat(_("The document %1$s has unsaved changes.\n\n"
				       "Do you want to save the document or discard the changes?"),
					   fname);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));

	if (ret == 0) {
		if (buf->isUnnamed()) {
			if (!writeAs(buf))
				return false;
		} else if (!menuWrite(buf))
			return false;
	} else if (ret == 2)
		return false;
		
	removeAutosaveFile(buf->fileName());

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


Buffer * BufferList::last()
{
	if (bstore.empty())
		return 0;
	return bstore.back();
}


Buffer * BufferList::getBuffer(unsigned int choice)
{
	if (choice >= bstore.size())
		return 0;
	return bstore[choice];
}


Buffer * BufferList::next(Buffer const * buf) const
{
	BOOST_ASSERT(buf);

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
	BOOST_ASSERT(buf);

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
			string writefile = addName(mastertmpdir, (*it)->getLatexName());
			(*it)->makeLaTeXFile(FileName(writefile), mastertmpdir,
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
	// Use ::assert to avoid a loop, BOOST_ASSERT ends up calling ::assert
	// compare with 0 to avoid pointer/interger comparison
	// ::assert(buf != 0);
	if (!buf)
		return;

	// No need to save if the buffer has not changed.
	if (buf->isClean())
		return;

	string const doc = buf->isUnnamed()
		? onlyFilename(buf->fileName()) : buf->fileName();

	lyxerr << to_utf8(
		bformat(_("LyX: Attempting to save document %1$s"), from_utf8(doc)))
		<< endl;

	// We try to save three places:
	// 1) Same place as document. Unless it is an unnamed doc.
	if (!buf->isUnnamed()) {
		string s = buf->fileName();
		s += ".emergency";
		lyxerr << "  " << s << endl;
		if (buf->writeFile(FileName(s))) {
			buf->markClean();
			lyxerr << to_utf8(_("  Save seems successful. Phew.")) << endl;
			return;
		} else {
			lyxerr << to_utf8(_("  Save failed! Trying...")) << endl;
		}
	}

	// 2) In HOME directory.
	string s = addName(package().home_dir().absFilename(), buf->fileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (buf->writeFile(FileName(s))) {
		buf->markClean();
		lyxerr << to_utf8(_("  Save seems successful. Phew.")) << endl;
		return;
	}

	lyxerr << to_utf8(_("  Save failed! Trying...")) << endl;

	// 3) In "/tmp" directory.
	// MakeAbsPath to prepend the current
	// drive letter on OS/2
	s = addName(package().temp_dir().absFilename(), buf->fileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (buf->writeFile(FileName(s))) {
		buf->markClean();
		lyxerr << to_utf8(_("  Save seems successful. Phew.")) << endl;
		return;
	}
	lyxerr << to_utf8(_("  Save failed! Bummer. Document is lost.")) << endl;
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


void BufferList::setCurrentAuthor(docstring const & name, docstring const & email)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		(*it)->params().authors().record(0, Author(name, email));
	}
}


} // namespace lyx
