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
#include "InsetIterator.h"
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

#include "LyX.h"
#include "Session.h"

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
using support::prefixIs;
using support::sum;
using support::makedir;


EmbeddedFile::EmbeddedFile(string const & file, string const & inzip_name,
	bool embed, Inset const * inset)
	: DocFileName(file, true), inzip_name_(inzip_name), embedded_(embed),
		valid_(true), inset_list_()
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
				FileName(buf->fileName()),
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
	if (embedded())
		return embeddedFile(buf);
	else
		return absFilename();
}


void EmbeddedFile::invalidate()
{
	// Clear inset_list_ because they will be registered again.
	inset_list_.clear();
	valid_ = false;
}


bool EmbeddedFile::extract(Buffer const * buf) const
{

	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	if (!fs::exists(emb_file))
		return false;

	// if external file already exists ...
	if (fs::exists(ext_file)) {
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
	try {
		// need to make directory?
		string path = onlyPath(ext_file);
		if (!fs::is_directory(path))
			makedir(const_cast<char*>(path.c_str()), 0755);
		fs::copy_file(emb_file, ext_file, false);
		return true;
	} catch (fs::filesystem_error const & fe) {
		Alert::error(_("Copy file failure"),
			 bformat(_("Cannot copy file %1$s to %2$s.\n"
				   "Please check whether the directory exists and is writeable."),
					from_utf8(emb_file), from_utf8(ext_file)));
		LYXERR(Debug::DEBUG) << "Fs error: " << fe.what() << endl;
	}
	return false;
}


bool EmbeddedFile::updateFromExternalFile(Buffer const * buf) const
{
	string ext_file = absFilename();
	string emb_file = embeddedFile(buf);

	if (!fs::exists(ext_file))
		return false;
	
	// if embedded file already exists ...
	if (fs::exists(emb_file)) {
		// no need to copy if the files are the same
		if (sum(*this) == sum(FileName(emb_file)))
			return true;
		// other wise, ask if overwrite
		int const ret = Alert::prompt(
			_("Update embedded file?"),
			bformat(_("Embeddedl file %1$s already exists, do you want to overwrite it"),
				from_utf8(ext_file)), 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret != 0)
			// if the user does not want to overwrite, we still consider it
			// a successful operation.
			return true;
	}
	// copy file
	try {
		// need to make directory?
		string path = onlyPath(emb_file);
		if (!fs::is_directory(path))
			makedir(const_cast<char*>(path.c_str()), 0755);
		fs::copy_file(ext_file, emb_file, false);
		return true;
	} catch (fs::filesystem_error const & fe) {
		Alert::error(_("Copy file failure"),
			 bformat(_("Cannot copy file %1$s to %2$s.\n"
				   "Please check whether the directory exists and is writeable."),
					from_utf8(ext_file), from_utf8(emb_file)));
		LYXERR(Debug::DEBUG) << "Fs error: " << fe.what() << endl;
	}
	return false;
}


bool EmbeddedFiles::enabled() const
{
	return buffer_->params().embedded;
}


bool EmbeddedFiles::enable(bool flag)
{
	if (enabled() != flag) {
		// if enable, copy all files to temppath()
		// if disable, extract all files
		if ((flag && !updateFromExternalFile()) || (!flag && !extract()))
			return false;
		// if operation is successful
		buffer_->markDirty();
		buffer_->params().embedded = flag;
	}
}


void EmbeddedFiles::registerFile(string const & filename,
	bool embed, Inset const * inset)
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
		// if the file is embedded, the embedded file should have exist
		// check for this to ensure that our logic is correct
		if (it->embedded())
			BOOST_ASSERT(fs::exists(it->embeddedFile(buffer_)));
		it->validate();
		return;
	}
	// try to be more careful
	file_list_.push_back(EmbeddedFile(abs_filename, 
		getInzipName(abs_filename), embed, inset));
	// validate if things are OK
	BOOST_ASSERT(fs::exists(file_list_.back().availableFile(buffer_)));
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
		// we do not update items that are manually inserted
		if (it->refCount() > 0)
			it->invalidate();

	for (InsetIterator it = inset_iterator_begin(buffer_->inset()); it; ++it)
		it->registerEmbeddedFiles(*buffer_, *this);

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

	::zipFiles(zipfile.toFilesystemEncoding(), filenames);
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


EmbeddedFiles::EmbeddedFileList::const_iterator EmbeddedFiles::find(std::string filename) const
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
		if (it->valid() && it->embedded())
			if(!it->extract(buffer_))
				return false;
	return true;
}


bool EmbeddedFiles::updateFromExternalFile() const
{
	EmbeddedFileList::const_iterator it = file_list_.begin();
	EmbeddedFileList::const_iterator it_end = file_list_.end();
	for (; it != it_end; ++it)
		if (it->valid() && it->embedded())
			if (!it->updateFromExternalFile(buffer_))
				return false;
	return true;
}


string const EmbeddedFiles::getInzipName(string const & abs_filename)
{
	// register a new one, using relative file path as inzip_name
	string inzip_name = to_utf8(makeRelPath(from_utf8(abs_filename),
		from_utf8(buffer_->fileName())));
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
		int embed;
		itmp >> embed;

		getline(is, tmp);
		if (tmp != "</file>") {
			lyxerr << "Invalid manifest file, lacking </file>" << endl;
			break;
		}

		files.registerFile(fname, embed);
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
			<< it->embedded() << '\n'
			<< "</file>\n";
	}
	os << "</manifest>\n";
	return os;
}

}
