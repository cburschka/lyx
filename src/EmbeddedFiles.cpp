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
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"
#include "support/FileZipListDir.h"

#include <sstream>
#include <fstream>
#include <utility>

using namespace std;
using namespace lyx::support;

namespace lyx {

/** Dir name used for ".." in the bundled file.

Under the lyx temp directory, content.lyx and its embedded files are usually
saved as

$temp/file.lyx
$temp/figure1.png     for ./figure1.png)
$temp/sub/figure2.png for ./sub/figure2.png)

This works fine for embedded files that are in the current or deeper directory
of the document directory, but not for files such as ../figures/figure.png.
A unique name $upDirName is chosen to represent .. in such filenames so that
'up' directories can be stored 'down' the directory tree:

$temp/$upDirName/figures/figure.png     for ../figures/figure.png
$temp/$upDirName/$upDirName/figure.png  for ../../figure.png

This name has to be fixed because it is used in lyx bundled .zip file.

Note that absolute files are not embeddable because there is no easy
way to put them under $temp.
*/
const std::string upDirName = "LyX.Embed.Dir.Up";

namespace Alert = frontend::Alert;

EmbeddedFile::EmbeddedFile(string const & file, std::string const & buffer_path)
	: DocFileName("", false), inzip_name_(""), embedded_(false), inset_list_()
{
	set(file, buffer_path);
}


void EmbeddedFile::set(std::string const & filename, std::string const & buffer_path)
{
	DocFileName::set(filename, buffer_path);
	if (filename.empty())
		return;

	inzip_name_ = to_utf8(makeRelPath(from_utf8(absFilename()),
			from_utf8(buffer_path)));
	// if inzip_name_ is an absolute path, this file is not embeddable
	if (FileName(inzip_name_).isAbsolute())
		inzip_name_ = "";
	// replace .. by upDirName
	if (prefixIs(inzip_name_, "."))
		inzip_name_ = subst(inzip_name_, "..", upDirName);
	LYXERR(Debug::FILES, "Create embedded file " << filename <<
		" with in zip name " << inzip_name_ << endl);
}


string EmbeddedFile::embeddedFile(Buffer const * buf) const
{
	BOOST_ASSERT(embeddable());
	string temp = buf->temppath();
	if (!suffixIs(temp, '/'))
		temp += '/';
	return temp + inzip_name_;
}


string EmbeddedFile::availableFile(Buffer const * buf) const
{
	return embedded() ? embeddedFile(buf) : absFilename();
}


void EmbeddedFile::addInset(Inset const * inset)
{
	if (inset != NULL)
		inset_list_.push_back(inset);
}


Inset const * EmbeddedFile::inset(int idx) const
{
	BOOST_ASSERT(idx < refCount());
	// some embedded file do not have a valid par iterator
	return inset_list_[idx];
}


void EmbeddedFile::setEmbed(bool embed)
{
	if (!embeddable() && embed) {
		Alert::error(_("Embedding failed."), bformat(
			_("Cannot embed file %1$s because its path is not relative to document path."),
			from_utf8(absFilename())));
		return;
	}
	embedded_ = embed;
}


bool EmbeddedFile::extract(Buffer const * buf) const
{
	BOOST_ASSERT(embeddable());

	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	FileName emb(emb_file);
	FileName ext(ext_file);

	if (!emb.exists())
		throw ExceptionMessage(ErrorException, _("Failed to extract file"),
			bformat(_("Cannot extract file '%1$s'.\n"
			"Source file %2$s does not exist"),
			from_utf8(outputFilename()), from_utf8(emb_file)));

	// if external file already exists ...
	if (ext.exists()) {
		// no need to copy if the files are the same
		if (checksum() == FileName(emb_file).checksum())
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
	if (!path.createPath()) {
		throw ExceptionMessage(ErrorException, _("Copy file failure"),
			bformat(_("Cannot create file path '%1$s'.\n"
			"Please check whether the path is writeable."),
			from_utf8(path.absFilename())));
		return false;
	}

	if (emb.copyTo(ext)) {
		LYXERR(Debug::FILES, "Extract file " << emb_file << " to " << ext_file << endl);
		return true;
	}

	throw ExceptionMessage(ErrorException, _("Copy file failure"),
		 bformat(_("Cannot copy file %1$s to %2$s.\n"
				 "Please check whether the directory exists and is writeable."),
				from_utf8(emb_file), from_utf8(ext_file)));
	return false;
}


bool EmbeddedFile::updateFromExternalFile(Buffer const * buf) const
{
	BOOST_ASSERT(embeddable());

	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	FileName emb(emb_file);
	FileName ext(ext_file);

	if (!ext.exists()) {
		// no need to update
		if (emb.exists())
			return true;
		// no external and internal file
		throw ExceptionMessage(ErrorException,
			_("Failed to embed file"),
			bformat(_("Failed to embed file %1$s.\n"
			   "Please check whether this file exists and is readable."),
				from_utf8(ext_file)));
	}

	// if embedded file already exists ...
	if (emb.exists()) {
		// no need to copy if the files are the same
		if (checksum() == FileName(emb_file).checksum())
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
		path.createPath();
	if (ext.copyTo(emb))
		return true;
	throw ExceptionMessage(ErrorException,
		_("Copy file failure"),
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


bool operator==(EmbeddedFile const & lhs, EmbeddedFile const & rhs)
{
	return lhs.absFilename() == rhs.absFilename()
		&& lhs.saveAbsPath() == rhs.saveAbsPath()
		&& lhs.embedded() == rhs.embedded();
}


bool operator!=(EmbeddedFile const & lhs, EmbeddedFile const & rhs)
{
	return !(lhs == rhs);
}


bool EmbeddedFiles::enabled() const
{
	return buffer_->params().embedded;
}


void EmbeddedFiles::enable(bool flag)
{
	if (enabled() != flag) {
		// update embedded file list
		update();
		// An exception may be thrown.
		if (flag)
			// if enable, copy all files to temppath()
			updateFromExternalFile();
		else
			// if disable, extract all files
			extractAll();
		// if operation is successful (no exception is thrown)
		buffer_->markDirty();
		buffer_->params().embedded = flag;
		if (flag)
			updateInsets();
	}
}


EmbeddedFile & EmbeddedFiles::registerFile(EmbeddedFile const & file, Inset const * inset)
{
	// try to find this file from the list
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->absFilename() == file.absFilename()) {
			if (it->embedded() != file.embedded()) {
				Alert::error(_("Wrong embedding status."),
					bformat(_("File %1$s is included in more than one insets, "
						"but with different embedding status. Assuming embedding status."),
						from_utf8(it->outputFilename())));
				it->setEmbed(true);
			}
			it->addInset(inset);
			return *it;
		}
	//
	file_list_.push_back(file);
	file_list_.back().addInset(inset);
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
			string file = it->embeddedFile(buffer_);
			if (!FileName(file).exists())
				throw ExceptionMessage(ErrorException, _("Failed to write file"),
					bformat(_("Embedded file %1$s does not exist. Did you tamper lyx temporary directory?"),
						it->displayName()));
			filenames.push_back(make_pair(file, it->inzipName()));
			LYXERR(Debug::FILES, "Writing file " << it->outputFilename()
				<< " as " << it->inzipName() << endl);
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
EmbeddedFiles::find(string filename) const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->absFilename() == filename || it->embeddedFile(buffer_) == filename)
			return it;
	return file_list_.end();
}


bool EmbeddedFiles::extractAll() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	int count_extracted = 0;
	int count_external = 0;
	for (; it != it_end; ++it)
		if (it->embedded()) {
			if(!it->extract(buffer_)) {
				throw ExceptionMessage(ErrorException,
					_("Failed to extract file"),
					bformat(_("Error: can not extract file %1$s.\n"), it->displayName()));
			} else
				count_extracted += 1;
		} else
			count_external += 1;
	docstring const msg = bformat(_("%1$d external or non-embeddable files are ignored.\n"
		"%2$d embedded files are extracted.\n"), count_external, count_extracted);
	Alert::information(_("Unpacking all files"), msg);
	return true;
}


bool EmbeddedFiles::updateFromExternalFile() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	int count_embedded = 0;
	int count_external = 0;
	for (; it != it_end; ++it)
		if (it->embedded()) {
			if (!it->updateFromExternalFile(buffer_)) {
				throw ExceptionMessage(ErrorException,
					_("Failed to embed file"),
					bformat(_("Error: can not embed file %1$s.\n"), it->displayName()));
				return false;
			} else
				count_embedded += 1;
		} else
			count_external += 1;
	docstring const msg = bformat(_("%1$d external or non-embeddable files are ignored.\n"
		"%2$d embeddable files are embedded.\n"), count_external, count_embedded);
	Alert::information(_("Packing all files"), msg);
	return true;
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
