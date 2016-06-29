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

#include "support/ExceptionMessage.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Package.h"

#include "support/lassert.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = lyx::frontend::Alert;


BufferList::BufferList()
{}


BufferList::~BufferList()
{
	BufferStorage::iterator it = binternal.begin();
	BufferStorage::iterator end = binternal.end();
	for (; it != end; ++it)
		delete (*it);
}


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
	// We may leak here, but we probably do not need to
	// shut down.
	LASSERT(buf, return);
	BufferStorage::iterator const it =
		find(bstore.begin(), bstore.end(), buf);
	if (it != bstore.end()) {
		Buffer * tmp = (*it);
		bstore.erase(it);
		LASSERT(tmp, return);
		delete tmp;
	}
}


Buffer * BufferList::newInternalBuffer(string const & s)
{
	Buffer * const buf = createNewBuffer(s);
	if (buf) {
		buf->setInternal(true);
		binternal.push_back(buf);
	}
	return buf;
}


Buffer * BufferList::newBuffer(string const & s)
{
	Buffer * const buf = createNewBuffer(s);
	if (buf) {
		LYXERR(Debug::INFO, "Assigning to buffer " << bstore.size());
		bstore.push_back(buf);
	}
	return buf;
}


Buffer * BufferList::createNewBuffer(string const & s)
{
	unique_ptr<Buffer> tmpbuf;
	try {
		tmpbuf = make_unique<Buffer>(s);
	} catch (ExceptionMessage const & message) {
		if (message.type_ == ErrorException) {
			Alert::error(message.title_, message.details_);
			exit(1);
		} else if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
			return 0;
		}
	}
	tmpbuf->params().useClassDefaults();
	return tmpbuf.release();
}


void BufferList::closeAll()
{
	while (!bstore.empty())
		release(bstore.front());
}


FileNameList BufferList::fileNames() const
{
	FileNameList nvec;
	BufferStorage::const_iterator it = bstore.begin();
	BufferStorage::const_iterator end = bstore.end();
	for (; it != end; ++it) {
		Buffer * buf = *it;
		nvec.push_back(buf->fileName());
	}
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
	// Something is wrong, but we can probably survive it.
	LASSERT(buf, return 0);

	if (bstore.empty())
		return 0;
	BufferStorage::const_iterator it = 
			find(bstore.begin(), bstore.end(), buf);
	LASSERT(it != bstore.end(), return 0);
	++it;
	Buffer * nextbuf = (it == bstore.end()) ? bstore.front() : *it;
	return nextbuf;
}


Buffer * BufferList::previous(Buffer const * buf) const
{
	// Something is wrong, but we can probably survive it.
	LASSERT(buf, return 0);

	if (bstore.empty())
		return 0;
	BufferStorage::const_iterator it = 
			find(bstore.begin(), bstore.end(), buf);
	LASSERT(it != bstore.end(), return 0);

	Buffer * previousbuf = (it == bstore.begin()) ? bstore.back() : *(it - 1);
	return previousbuf;
}


void BufferList::updateIncludedTeXfiles(string const & masterTmpDir,
					OutputParams const & runparams_in)
{
	OutputParams runparams = runparams_in;
	runparams.is_child = true;
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it) {
		if (!(*it)->isDepClean(masterTmpDir)) {
			string writefile = addName(masterTmpDir, (*it)->latexName());
			(*it)->makeLaTeXFile(FileName(writefile), masterTmpDir,
					     runparams, Buffer::OnlyBody);
			(*it)->markDepClean(masterTmpDir);
		}
	}
	runparams.is_child = false;
}


void BufferList::emergencyWriteAll()
{
	BufferStorage::const_iterator it = bstore.begin();
	BufferStorage::const_iterator const en = bstore.end();
	for (; it != en; ++it)
		 (*it)->emergencyWrite();
}


bool BufferList::exists(FileName const & fname) const
{
	return getBuffer(fname) != 0;
}


 bool BufferList::isLoaded(Buffer const * b) const
{
	if (!b)
		return false;
	BufferStorage::const_iterator cit =
		find(bstore.begin(), bstore.end(), b);
	return cit != bstore.end();
}


bool BufferList::isOthersChild(Buffer * parent, Buffer * child)
{
	LASSERT(parent, return false);
	LASSERT(child, return false);
	LASSERT(parent->isChild(child), return false);
	
	// Does child document have a different parent?
	Buffer const * parent_ = child->parent();
	if (parent_ && parent_ != parent)
		return true;

	for(Buffer * buf : bstore)
		if (buf != parent && buf->isChild(child))
			return true;
	return false;
}


Buffer * BufferList::getBuffer(support::FileName const & fname, bool internal) const
{
	// 1) cheap test, using string comparison of file names
	for (Buffer * b : bstore)
		if (b->fileName() == fname)
			return b;
	// 2) possibly expensive test, using equivalence test of file names
	for (Buffer * b : bstore)
		if (equivalent(b->fileName(), fname))
			return b;
	if (internal) {
		// 1) cheap test, using string comparison of file names
		for (Buffer * b : binternal)
			if (b->fileName() == fname)
				return b;
		// 2) possibly expensive test, using equivalence test of file names
		for (Buffer * b : binternal)
			if (equivalent(b->fileName(), fname))
				return b;
	}
	return 0;
}


Buffer * BufferList::getBufferFromTmp(string const & s)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it < end; ++it) {
		if (prefixIs(s, (*it)->temppath())) {
			// check whether the filename matches the master
			string const master_name = (*it)->latexName();
			if (suffixIs(s, master_name))
				return *it;
			// if not, try with the children
			ListOfBuffers clist = (*it)->getDescendents();
			ListOfBuffers::const_iterator cit = clist.begin();
			ListOfBuffers::const_iterator cend = clist.end();
			for (; cit != cend; ++cit) {
				string const mangled_child_name = DocFileName(
					changeExtension((*cit)->absFileName(),
						".tex")).mangledFileName();
				if (suffixIs(s, mangled_child_name))
					return *cit;
			}
		}
	}
	return 0;
}


void BufferList::recordCurrentAuthor(Author const & author)
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it)
		(*it)->params().authors().recordCurrentAuthor(author);
}


void BufferList::updatePreviews()
{
	BufferStorage::iterator it = bstore.begin();
	BufferStorage::iterator end = bstore.end();
	for (; it != end; ++it)
		(*it)->updatePreviews();
}


int BufferList::bufferNum(FileName const & fname) const
{
	FileNameList const buffers(fileNames());
	FileNameList::const_iterator cit =
		find(buffers.begin(), buffers.end(), fname);
	if (cit == buffers.end())
		return 0;
	return int(cit - buffers.begin());
}


void BufferList::changed(bool update_metrics) const
{
	BufferStorage::const_iterator it = bstore.begin();
	BufferStorage::const_iterator end = bstore.end();
	for (; it != end; ++it)
		(*it)->changed(update_metrics);
	it = binternal.begin();
	end = binternal.end();
	for (; it != end; ++it)
		(*it)->changed(update_metrics);
}


} // namespace lyx
