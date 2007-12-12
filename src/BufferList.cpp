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
#include "Session.h"
#include "LyX.h"
#include "output_latex.h"
#include "ParagraphList.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Package.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

using boost::bind;

using namespace std;
using namespace lyx::support;

namespace lyx {

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
	LYXERR(Debug::INFO, "Assigning to buffer " << bstore.size());
	bstore.push_back(tmpbuf.get());
	return tmpbuf.release();
}


void BufferList::closeAll()
{
	while (!bstore.empty())
		release(bstore.front());
}


vector<string> const BufferList::getFileNames() const
{
	vector<string> nvec;
	transform(bstore.begin(), bstore.end(),
		  back_inserter(nvec),
		  boost::bind(&Buffer::absFileName, _1));
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


void BufferList::updateIncludedTeXfiles(string const & masterTmpDir,
					OutputParams const & runparams)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		if (!(*it)->isDepClean(masterTmpDir)) {
			string writefile = addName(masterTmpDir, (*it)->latexName());
			(*it)->makeLaTeXFile(FileName(writefile), masterTmpDir,
					     runparams, false);
			(*it)->markDepClean(masterTmpDir);
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
		? onlyFilename(buf->absFileName()) : buf->absFileName();

	lyxerr << to_utf8(
		bformat(_("LyX: Attempting to save document %1$s"), from_utf8(doc)))
		<< endl;

	// We try to save three places:
	// 1) Same place as document. Unless it is an unnamed doc.
	if (!buf->isUnnamed()) {
		string s = buf->absFileName();
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
	string s = addName(package().home_dir().absFilename(), buf->absFileName());
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
	s = addName(package().temp_dir().absFilename(), buf->absFileName());
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
			    bind(&Buffer::absFileName, _1),
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
			     bind(&Buffer::absFileName, _1),
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
	for (; it != end; ++it)
		(*it)->params().authors().record(0, Author(name, email));
}


int BufferList::bufferNum(std::string const & name) const
{
	vector<string> buffers = getFileNames();
	vector<string>::const_iterator cit =
		std::find(buffers.begin(), buffers.end(), name);
	if (cit == buffers.end())
		return 0;
	return int(cit - buffers.begin());
}


} // namespace lyx
