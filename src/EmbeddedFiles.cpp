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
#include "Paragraph.h"
#include "ParIterator.h"
#include "debug.h"
#include "gettext.h"
#include "Format.h"

#include "frontends/alert.h"

#include <boost/filesystem/operations.hpp>

#include "support/filetools.h"
#include "support/fs_extras.h"
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

namespace fs = boost::filesystem;
namespace Alert = frontend::Alert;

using support::FileName;
using support::DocFileName;
using support::makeAbsPath;
using support::addName;
using support::onlyPath;
using support::absolutePath;
using support::onlyFilename;
using support::makeRelPath;
using support::changeExtension;
using support::bformat;
using support::zipFiles;
using support::prefixIs;
using support::sum;


EmbeddedFile::EmbeddedFile(string const & file, string const & inzip_name,
	STATUS status, ParConstIterator const & pit)
	: DocFileName(file, true), inzip_name_(inzip_name), status_(status),
		valid_(true), par_it_(pit)
{}


string EmbeddedFile::embeddedFile(Buffer const * buf) const
{
	return addName(buf->temppath(), inzip_name_);
}


int const EmbeddedFile::parID() const
{
	// some embedded file do not have a valid par iterator
	return par_it_ == ParConstIterator() ? 0 : par_it_->id();
}


void EmbeddedFile::setParIter(ParConstIterator const & pit)
{
	par_it_ = pit;
}


string EmbeddedFile::availableFile(Buffer const * buf) const
{
	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);
	if (status_ == AUTO) {
		// use external file first
		if (fs::exists(ext_file))
			return ext_file;
		else if (fs::exists(emb_file))
			return emb_file;
		else
			return string();
	} else if (status_ == EMBEDDED) {
		// use embedded file first
		if (fs::exists(emb_file))
			return emb_file;
		else if (fs::exists(ext_file))
			return ext_file;
		else
			return string();
	} else
		return string();
}


bool EmbeddedFile::extract(Buffer const * buf) const
{
	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);
	bool copyFile = false;
	// both files exist, are different, and in EMBEDDED status
	if (fs::exists(ext_file) && fs::exists(emb_file) && status_ == EMBEDDED
		&& sum(*this) != sum(FileName(emb_file))) {
		int const ret = Alert::prompt(
			_("Overwrite external file?"),
			bformat(_("External file %1$s already exists, do you want to overwrite it"),
				from_utf8(ext_file)), 1, 1, _("&Overwrite"), _("&Cancel"));
		copyFile = ret == 0;
	}
	// copy file in the previous case, and a new case
	if (copyFile || (!fs::exists(ext_file) && fs::exists(emb_file))) {
		try {
			fs::copy_file(emb_file, ext_file, false);
			return true;
		} catch (fs::filesystem_error const & fe) {
			Alert::error(_("Copy file failure"),
				 bformat(_("Cannot copy file %1$s to %2$s.\n"
					   "Please check whether the directory exists and is writeable."),
						from_utf8(emb_file), from_utf8(ext_file)));
			LYXERR(Debug::DEBUG) << "Fs error: " << fe.what() << endl;
		}
	}
	return false;
}

 
bool EmbeddedFiles::enabled() const
{
	return buffer_->params().embedded;
}


void EmbeddedFiles::enable(bool flag)
{
	if (enabled() != flag) {
		// if disable embedding, first extract all embedded files
		if (flag || (!flag && extractAll())) {
			// file will be changed
			buffer_->markDirty();
			buffer_->params().embedded = flag;
		}
	}
}


void EmbeddedFiles::registerFile(string const & filename,
	EmbeddedFile::STATUS status, ParConstIterator const & pit)
{
	string abs_filename = makeAbsPath(filename, buffer_->filePath()).absFilename();
	// try to find this file from the list
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->absFilename() == abs_filename)
			break;
	// find this filename
	if (it != file_list_.end()) {
		it->setParIter(pit);
		it->setStatus(status);
		it->validate();
		return;
	}
	file_list_.push_back(EmbeddedFile(abs_filename, 
		getInzipName(abs_filename), status, pit));
}


void EmbeddedFiles::update()
{
	// invalidate all files, obsolete files will then not be validated by the
	// following document scan. These files will still be kept though, because
	// they may be added later and their embedding status will be meaningful
	// again (thinking of cut/paste of an InsetInclude).
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		// Only AUTO files will be updated. If the status of a file is EMBEDDED, 
		// it will be embedded even if it is not referred by a document.
		if (it->status() == EmbeddedFile::AUTO)
			it->invalidate();

	ParIterator pit = buffer_->par_iterator_begin();
	ParIterator pit_end = buffer_->par_iterator_end();
	for (; pit != pit_end; ++pit) {
		// For each paragraph, traverse its insets and register embedded files
		InsetList::const_iterator iit = pit->insetlist.begin();
		InsetList::const_iterator iit_end = pit->insetlist.end();
		for (; iit != iit_end; ++iit) {
			Inset & inset = *iit->inset;
			inset.registerEmbeddedFiles(*buffer_, *this, pit);
		}
	}
	LYXERR(Debug::FILES) << "Manifest updated: " << endl
		<< *this
		<< "End Manifest" << endl;
}


bool EmbeddedFiles::write(DocFileName const & filename)
{
	// file in the temporary path has the content
	string const content = FileName(addName(buffer_->temppath(),
		onlyFilename(filename.toFilesystemEncoding()))).toFilesystemEncoding();

	// get a file list and write a manifest file
	vector<pair<string, string> > filenames;
	string const manifest = FileName(
		addName(buffer_->temppath(), "manifest.txt")).toFilesystemEncoding();

	// write a manifest file
	ofstream os(manifest.c_str());
	os << *this;
	os.close();
	// prepare list of embedded file
	EmbeddedFileList::iterator it = file_list_.begin();
	EmbeddedFileList::iterator it_end = file_list_.end();
	for (; it != it_end; ++it) {
		if (it->valid() && it->embedded()) {
			string file = it->availableFile(buffer_);
			if (file.empty())
				lyxerr << "File " << it->absFilename() << " does not exist. Skip embedding it. " << endl;
			else
				filenames.push_back(make_pair(file, it->inzipName()));
		}
	}
	// add filename (.lyx) and manifest to filenames
	filenames.push_back(make_pair(content, onlyFilename(filename.toFilesystemEncoding())));
	filenames.push_back(make_pair(manifest, "manifest.txt"));
	// write a zip file with all these files. Write to a temp file first, to
	// avoid messing up the original file in case something goes terribly wrong.
	DocFileName zipfile(addName(buffer_->temppath(),
		onlyFilename(changeExtension(
			filename.toFilesystemEncoding(), ".zip"))));

	zipFiles(zipfile, filenames);
	// copy file back
	try {
		fs::copy_file(zipfile.toFilesystemEncoding(), filename.toFilesystemEncoding(), false);
	} catch (fs::filesystem_error const & fe) {
		Alert::error(_("Save failure"),
				 bformat(_("Cannot create file %1$s.\n"
					   "Please check whether the directory exists and is writeable."),
					 from_utf8(filename.absFilename())));
		LYXERR(Debug::DEBUG) << "Fs error: " << fe.what() << endl;
	}
	return true;
}


bool EmbeddedFiles::extractAll() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	// FIXME: the logic here is hard to decide, we should allow cancel for
	// 'do not overwrite' this file, and cancel for 'cancel extract all files'.
	// I am not sure how to do this now.
	for (; it != it_end; ++it)
		if (it->valid() && it->status() != EmbeddedFile::EXTERNAL)
			it->extract(buffer_);
	return true;
}


string const EmbeddedFiles::getInzipName(string const & abs_filename)
{
	// register a new one, using relative file path as inzip_name
	string inzip_name = to_utf8(makeRelPath(from_utf8(abs_filename),
		from_utf8(buffer_->fileName())));
	// if inzip_name is an absolute path, use filename only to avoid
	// leaking of filesystem information in inzip_name
	if (absolutePath(inzip_name) || prefixIs(inzip_name, ".."))
		inzip_name = onlyFilename(inzip_name);
	// if this name has been used...
	// use _1_name, _2_name etc
	string tmp = inzip_name;
	EmbeddedFileList::iterator it;
	EmbeddedFileList::iterator it_end = file_list_.end();
	bool unique_name = false;
	while (!unique_name) {
		unique_name = true;
		size_t i = 0;
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


istream & operator>> (istream & is, EmbeddedFiles & files)
{
	files.clear();
	string tmp;
	getline(is, tmp);
	// get version
	istringstream itmp(tmp);
	int version;
	itmp.ignore(string("# LyX manifest version ").size());
	itmp >> version;

	if (version != 1) {
		lyxerr << "This version of LyX can only read LyX manifest version 1" << endl;
		return is;
	}

	getline(is, tmp);
	if (tmp != "<manifest>") {
		lyxerr << "Invalid manifest file, lacking <manifest>" << endl;
		return is;
	}
	// manifest file may be messed up, be carefully
	while (is.good()) {
		getline(is, tmp);
		if (tmp != "<file>")
			break;

		string fname;
		getline(is, fname);
		string inzip_name;
		getline(is, inzip_name);
		getline(is, tmp);
		istringstream itmp(tmp);
		int status;
		itmp >> status;

		getline(is, tmp);
		if (tmp != "</file>") {
			lyxerr << "Invalid manifest file, lacking </file>" << endl;
			break;
		}

		files.registerFile(fname, static_cast<EmbeddedFile::STATUS>(status));
	};
	// the last line must be </manifest>
	if (tmp != "</manifest>") {
		lyxerr << "Invalid manifest file, lacking </manifest>" << endl;
		return is;
	}
	return is;
}


ostream & operator<< (ostream & os, EmbeddedFiles const & files)
{
	// store a version so that operator >> can read later versions
	// using version information.
	os << "# lyx manifest version 1\n";
	os << "<manifest>\n";
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files.end();
	for (; it != it_end; ++it) {
		if (!it->valid())
			continue;
		// use differnt lines to make reading easier.
		os << "<file>\n"
			// save the relative path
			<< to_utf8(makeRelPath(from_utf8(it->absFilename()),
				from_utf8(files.buffer_->filePath()))) << '\n'
			<< it->inzipName() << '\n'
			<< it->status() << '\n'
			<< "</file>\n";
	}
	os << "</manifest>\n";
	return os;
}

}
