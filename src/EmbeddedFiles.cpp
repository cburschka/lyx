// -*- C++ -*-
/**
 * \file EmbeddedFiles.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#include <config.h>

#include "EmbeddedFiles.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "ErrorList.h"
#include "Format.h"
#include "InsetIterator.h"
#include "Lexer.h"
#include "LyX.h"
#include "Paragraph.h"
#include "Session.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/convert.h"
#include "support/lyxlib.h"
#include "support/lstrings.h"

#include <sstream>
#include <fstream>
#include <utility>

using std::ofstream;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::make_pair;
using std::istream;
using std::ostream;
using std::getline;
using std::istringstream;

namespace lyx {

namespace Alert = frontend::Alert;

using support::FileName;
using support::DocFileName;
using support::makeAbsPath;
using support::addName;
using support::absolutePath;
using support::onlyFilename;
using support::makeRelPath;
using support::changeExtension;
using support::bformat;
using support::prefixIs;
using support::sum;
using support::makedir;


EmbeddedFile::EmbeddedFile(string const & file, string const & inzip_name,
	bool embed, Inset const * inset)
	: DocFileName(file, true), inzip_name_(inzip_name), embedded_(embed),
		inset_list_()
{
	if (inset != NULL)
		inset_list_.push_back(inset);
}


string EmbeddedFile::embeddedFile(Buffer const * buf) const
{
	return addName(buf->temppath(), inzip_name_);
}


void EmbeddedFile::addInset(Inset const * inset)
{
	inset_list_.push_back(inset);
}


Inset const * EmbeddedFile::inset(int idx) const
{
	BOOST_ASSERT(idx < refCount());
	// some embedded file do not have a valid par iterator
	return inset_list_[idx];
}


void EmbeddedFile::saveBookmark(Buffer const * buf, int idx) const
{
	Inset const * ptr = inset(idx);
	// This might not be the most efficient method ... 
	for (InsetIterator it = inset_iterator_begin(buf->inset()); it; ++it)
		if (&(*it) == ptr) {
			// this is basically BufferView::saveBookmark(0)
			LyX::ref().session().bookmarks().save(
				FileName(buf->absFileName()),
				it.bottom().pit(),
				it.bottom().pos(),
				it.paragraph().id(),
				it.pos(),
				0
			);
		}
	// this inset can not be located. There is something wrong that needs
	// to be fixed.
	BOOST_ASSERT(true);
}


string EmbeddedFile::availableFile(Buffer const * buf) const
{
	return embedded() ? embeddedFile(buf) : absFilename();
}


bool EmbeddedFile::extract(Buffer const * buf) const
{
	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	FileName emb(emb_file);
	FileName ext(ext_file);

	if (!emb.exists())
		return false;

	// if external file already exists ...
	if (ext.exists()) {
		// no need to copy if the files are the same
		if (sum(*this) == sum(FileName(emb_file)))
			return true;
		// otherwise, ask if overwrite
		int ret = Alert::prompt(
			_("Overwrite external file?"),
			bformat(_("External file %1$s already exists, do you want to overwrite it"),
				from_utf8(ext_file)), 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret != 0)
			// if the user does not want to overwrite, we still consider it
			// a successful operation.
			return true;
	}
	// copy file

	// need to make directory?
	FileName path = ext.onlyPath();
	if (!path.isDirectory())
		makedir(const_cast<char*>(path.absFilename().c_str()), 0755);
	if (emb.copyTo(ext))
		return true;
	Alert::error(_("Copy file failure"),
		 bformat(_("Cannot copy file %1$s to %2$s.\n"
				 "Please check whether the directory exists and is writeable."),
				from_utf8(emb_file), from_utf8(ext_file)));
	//LYXERR(Debug::DEBUG, "Fs error: " << fe.what());
	return false;
}


bool EmbeddedFile::updateFromExternalFile(Buffer const * buf) const
{
	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	FileName emb(emb_file);
	FileName ext(ext_file);

	if (!ext.exists())
		return false;
	
	// if embedded file already exists ...
	if (emb.exists()) {
		// no need to copy if the files are the same
		if (sum(*this) == sum(FileName(emb_file)))
			return true;
		// other wise, ask if overwrite
		int const ret = Alert::prompt(
			_("Update embedded file?"),
			bformat(_("Embedded file %1$s already exists, do you want to overwrite it"),
				from_utf8(ext_file)), 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret != 0)
			// if the user does not want to overwrite, we still consider it
			// a successful operation.
			return true;
	}
	// copy file
	// need to make directory?
	FileName path = emb.onlyPath();
	if (!path.isDirectory())
		makedir(const_cast<char*>(path.absFilename().c_str()), 0755);
	if (ext.copyTo(emb))
		return true;
	Alert::error(_("Copy file failure"),
		 bformat(_("Cannot copy file %1$s to %2$s.\n"
			   "Please check whether the directory exists and is writeable."),
				from_utf8(ext_file), from_utf8(emb_file)));
	//LYXERR(Debug::DEBUG, "Fs error: " << fe.what());
	return false;
}


void EmbeddedFile::updateInsets(Buffer const * buf) const
{
	vector<Inset const *>::const_iterator it = inset_list_.begin();
	vector<Inset const *>::const_iterator it_end = inset_list_.end();
	for (; it != it_end; ++it)
		const_cast<Inset *>(*it)->updateEmbeddedFile(*buf, *this);
}


bool EmbeddedFiles::enabled() const
{
	return buffer_->params().embedded;
}


void EmbeddedFiles::enable(bool flag)
{
	if (enabled() != flag) {
		// if enable, copy all files to temppath()
		// if disable, extract all files
		if ((flag && !updateFromExternalFile()) || (!flag && !extract()))
			return;
		// if operation is successful
		buffer_->markDirty();
		buffer_->params().embedded = flag;
		if (flag)
			updateInsets();
	}
}


EmbeddedFile & EmbeddedFiles::registerFile(string const & filename,
	bool embed, Inset const * inset, string const & inzipName)
{
	// filename can be relative or absolute, translate to absolute filename
	string abs_filename = makeAbsPath(filename, buffer_->filePath()).absFilename();
	// try to find this file from the list
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->absFilename() == abs_filename || it->embeddedFile(buffer_) == abs_filename)
			break;
	// find this filename, keep the original embedding status
	if (it != file_list_.end()) {
		it->addInset(inset);
		return *it;
	}
	//
	file_list_.push_back(EmbeddedFile(abs_filename, 
		getInzipName(abs_filename, inzipName), embed, inset));
	return file_list_.back();
}


void EmbeddedFiles::update()
{
	file_list_.clear();

	for (InsetIterator it = inset_iterator_begin(buffer_->inset()); it; ++it)
		it->registerEmbeddedFiles(*buffer_, *this);
}


bool EmbeddedFiles::writeFile(DocFileName const & filename)
{
	// file in the temporary path has the content
	string const content = FileName(addName(buffer_->temppath(),
		"content.lyx")).toFilesystemEncoding();

	vector<pair<string, string> > filenames;
	// add content.lyx to filenames
	filenames.push_back(make_pair(content, "content.lyx"));
	// prepare list of embedded file
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it) {
		if (it->embedded()) {
			string file = it->availableFile(buffer_);
			if (file.empty())
				lyxerr << "File " << it->absFilename() << " does not exist. Skip embedding it. " << endl;
			else
				filenames.push_back(make_pair(file, it->inzipName()));
		}
	}
	// write a zip file with all these files. Write to a temp file first, to
	// avoid messing up the original file in case something goes terribly wrong.
	DocFileName zipfile(addName(buffer_->temppath(),
		onlyFilename(changeExtension(
			filename.toFilesystemEncoding(), ".zip"))));

	::zipFiles(zipfile.toFilesystemEncoding(), filenames);
	// copy file back
	if (!zipfile.copyTo(filename)) {
		Alert::error(_("Save failure"),
				 bformat(_("Cannot create file %1$s.\n"
					   "Please check whether the directory exists and is writeable."),
					 from_utf8(filename.absFilename())));
		//LYXERR(Debug::DEBUG, "Fs error: " << fe.what());
	}
	return true;
}


EmbeddedFiles::EmbeddedFileList::const_iterator
EmbeddedFiles::find(std::string filename) const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->absFilename() == filename || it->embeddedFile(buffer_) == filename)	
			return it;
	return file_list_.end();
}


bool EmbeddedFiles::extract() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->embedded())
			if(!it->extract(buffer_))
				return false;
	return true;
}


bool EmbeddedFiles::updateFromExternalFile() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->embedded())
			if (!it->updateFromExternalFile(buffer_))
				return false;
	return true;
}


string const EmbeddedFiles::getInzipName(string const & abs_filename, string const & name)
{
	// register a new one, using relative file path as inzip_name
	string inzip_name = name;
	if (name.empty())
		inzip_name = to_utf8(makeRelPath(from_utf8(abs_filename),
			from_utf8(buffer_->filePath())));
	// if inzip_name is an absolute path, use filename only to avoid
	// leaking of filesystem information in inzip_name
	// The second case covers cases '../path/file' and '.'
	if (absolutePath(inzip_name) || prefixIs(inzip_name, "."))
		inzip_name = onlyFilename(abs_filename);
	// if this name has been used...
	// use _1_name, _2_name etc
	string tmp = inzip_name;
	EmbeddedFileList::iterator it;
	EmbeddedFileList::iterator it_end = file_list_.end();
	bool unique_name = false;
	size_t i = 0;
	while (!unique_name) {
		unique_name = true;
		if (i > 0)
			inzip_name = convert<string>(i) + "_" + tmp;
		it = file_list_.begin();
		for (; it != it_end; ++it)
			if (it->inzipName() == inzip_name) {
				unique_name = false;
				++i;
				break;
			}
	}
	return inzip_name;
}


void EmbeddedFiles::updateInsets() const
{
	EmbeddedFiles::EmbeddedFileList::const_iterator it = begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = end();
	for (; it != it_end; ++it)
		if (it->refCount() > 0)
			it->updateInsets(buffer_);
}


}
