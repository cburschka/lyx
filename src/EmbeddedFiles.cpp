// -*- C++ -*-
/**
 * \file EmbeddedFileList.cpp
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

#include <boost/assert.hpp>

#include <sstream>
#include <fstream>
#include <utility>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;

EmbeddedFile::EmbeddedFile(string const & file, std::string const & buffer_path)
	: DocFileName("", false), embedded_(false), inset_list_()
{
	set(file, buffer_path);
}


void EmbeddedFile::set(std::string const & filename, std::string const & buffer_path)
{
	DocFileName::set(filename, buffer_path);
	if (filename.empty())
		return;

	if (!buffer_path.empty())
		inzip_name_ = calcInzipName(buffer_path);
}


void EmbeddedFile::setInzipName(std::string const & name)
{
	if (name.empty() || name == inzip_name_)
		return;

	// an enabled EmbeededFile should have this problem handled
	BOOST_ASSERT(!enabled());
	// file will be synced when it is enabled
	inzip_name_ = name;
}


string EmbeddedFile::embeddedFile() const
{
	BOOST_ASSERT(enabled());
	return temp_path_ + inzip_name_;
}


FileName EmbeddedFile::availableFile() const
{
	if (enabled() && embedded())
		return FileName(embeddedFile());
	else
		return *this;
}


string EmbeddedFile::latexFilename(std::string const & buffer_path) const
{
	return (enabled() && embedded()) ? inzip_name_ : relFilename(buffer_path);
}


void EmbeddedFile::addInset(Inset const * inset)
{
	if (inset != NULL)
		inset_list_.push_back(inset);
}


void EmbeddedFile::setEmbed(bool embed)
{
	embedded_ = embed;
}


void EmbeddedFile::enable(bool flag, Buffer const * buf, bool updateFile)
{
	// This function will be called when
	// 1. through EmbeddedFiles::enable() when a file is read. Files
	//    should be in place so no updateFromExternalFile or extract()
	//    should be called. (updateFile should be false in this case).
	// 2. through menu item enable/disable. updateFile should be true.
	// 3. A single embedded file is added or modified. updateFile
	//    can be true or false.
	LYXERR(Debug::FILES, (flag ? "Enable" : "Disable") 
		<< " " << absFilename() 
		<< (updateFile ? " (update file)." : " (no update)."));

	if (flag) {
		temp_path_ = buf->temppath();
		if (!suffixIs(temp_path_, '/'))
			temp_path_ += '/';
		if (embedded() && updateFile)
			updateFromExternalFile();
	} else {
		// when a new embeddeed file is created, it is not enabled, and 
		// there is no need to extract.
		if (enabled() && embedded() && updateFile)
			extract();
		temp_path_ = "";
	}
}


bool EmbeddedFile::extract() const
{
	BOOST_ASSERT(enabled());

	string ext_file = absFilename();
	string emb_file = embeddedFile();

	FileName emb(emb_file);
	FileName ext(ext_file);

	if (!emb.exists()) {
		if (ext.exists())
			return true;
		throw ExceptionMessage(ErrorException, _("Failed to extract file"),
			bformat(_("Cannot extract file '%1$s'.\n"
			"Source file %2$s does not exist"),
			from_utf8(outputFilename()), from_utf8(emb_file)));
	}

	// if external file already exists ...
	if (ext.exists()) {
		// no need to copy if the files are the same
		if (checksum() == FileName(emb_file).checksum())
			return true;
		// otherwise, ask if overwrite
		int ret = Alert::prompt(
			_("Overwrite external file?"),
			bformat(_("External file %1$s already exists, do you want to overwrite it?"),
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


bool EmbeddedFile::updateFromExternalFile() const
{
	BOOST_ASSERT(enabled());

	string ext_file = absFilename();
	string emb_file = embeddedFile();

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


EmbeddedFile EmbeddedFile::copyTo(Buffer const * buf)
{
	EmbeddedFile file = EmbeddedFile(absFilename(), buf->filePath());
	file.setEmbed(embedded());
	file.enable(buf->embedded(), buf, false);
	
	// use external file.
	if (!embedded())
		return file;

	LYXERR(Debug::FILES, "Copy " << availableFile()
		<< " to " << file.availableFile());

	FileName from_file = availableFile();
	FileName to_file = file.availableFile();

	if (!from_file.exists()) {
		// no from file
		throw ExceptionMessage(ErrorException,
			_("Failed to copy embedded file"),
			bformat(_("Failed to embed file %1$s.\n"
			   "Please check whether the source file is available"),
				from_utf8(absFilename())));
		file.setEmbed(false);
		return file;
	}

	// if destination file already exists ...
	if (to_file.exists()) {
		// no need to copy if the files are the same
		if (checksum() == to_file.checksum())
			return file;
		// other wise, ask if overwrite
		int const ret = Alert::prompt(
			_("Update embedded file?"),
			bformat(_("Embedded file %1$s already exists, do you want to overwrite it"),
				from_utf8(to_file.absFilename())), 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret != 0)
			// if the user does not want to overwrite, we still consider it
			// a successful operation.
			return file;
	}
	// copy file
	// need to make directory?
	FileName path = to_file.onlyPath();
	if (!path.isDirectory())
		path.createPath();
	if (from_file.copyTo(to_file))
		return file;
	throw ExceptionMessage(ErrorException,
		_("Copy file failure"),
		bformat(_("Cannot copy file %1$s to %2$s.\n"
			   "Please check whether the directory exists and is writeable."),
				from_utf8(from_file.absFilename()), from_utf8(to_file.absFilename())));
	return file;
}


void EmbeddedFile::updateInsets() const
{
	vector<Inset const *>::const_iterator it = inset_list_.begin();
	vector<Inset const *>::const_iterator it_end = inset_list_.end();
	for (; it != it_end; ++it)
		const_cast<Inset *>(*it)->updateEmbeddedFile(*this);
}


bool EmbeddedFile::isReadableFile() const
{
	return availableFile().isReadableFile();
}


unsigned long EmbeddedFile::checksum() const
{
	return availableFile().checksum();
}

/**
Under the lyx temp directory, content.lyx and its embedded files are usually
saved as

$temp/$embDirName/file.lyx
$temp/$embDirName/figure1.png     for ./figure1.png)
$temp/$embDirName/sub/figure2.png for ./sub/figure2.png)

This works fine for embedded files that are in the current or deeper directory
of the document directory, but not for files such as ../figures/figure.png.
A unique name $upDirName is chosen to represent .. in such filenames so that
'up' directories can be stored 'down' the directory tree:

$temp/$embDirName/$upDirName/figures/figure.png     for ../figures/figure.png
$temp/$embDirName/$upDirName/$upDirName/figure.png  for ../../figure.png

This name has to be fixed because it is used in lyx bundled .zip file.

Using a similar trick, we use $absDirName for absolute path so that
an absolute filename can be saved as

$temp/$embDirName/$absDirName/a/absolute/path for /a/absolute/path

FIXME:
embDirName is set to . so that embedded layout and class files can be
used directly. However, putting all embedded files directly under
the temp directory may lead to file conflicts. For example, if a user
embeds a file blah.log in blah.lyx, it will be replaced when
'latex blah.tex' is called.
*/
const std::string embDirName = ".";
const std::string upDirName = "LyX.Embed.Dir.Up";
const std::string absDirName = "LyX.Embed.Dir.Abs";
const std::string driveName = "LyX.Embed.Drive";
const std::string spaceName = "LyX.Embed.Space";

std::string EmbeddedFile::calcInzipName(std::string const & buffer_path)
{
	string inzipName = to_utf8(makeRelPath(from_utf8(absFilename()),
			from_utf8(buffer_path)));
	
	if (FileName(inzipName).isAbsolute())
		inzipName = absDirName + '/' + inzipName;

	// replace .. by upDirName
	if (prefixIs(inzipName, "."))
		inzipName = subst(inzipName, "..", upDirName);
	// replace special characters by their value
	inzipName = subst(inzipName, ":", driveName);
	inzipName = subst(inzipName, " ", spaceName);

	// to avoid name conflict between $docu_path/file and $temp_path/file
	// embedded files are in a subdirectory of $temp_path.
	inzipName = embDirName + '/' + inzipName;
	return inzipName;
}


void EmbeddedFile::syncInzipFile(std::string const & buffer_path)
{
	BOOST_ASSERT(enabled());
	string old_emb_file = temp_path_ + '/' + inzip_name_;
	FileName old_emb(old_emb_file);

	if (!old_emb.exists())
		throw ExceptionMessage(ErrorException, _("Failed to open file"),
			bformat(_("Embedded file %1$s does not exist. Did you tamper lyx temporary directory?"),
				old_emb.displayName()));

	string new_inzip_name = calcInzipName(buffer_path);
	if (new_inzip_name == inzip_name_)
		return;

	LYXERR(Debug::FILES, " OLD ZIP " << old_emb_file <<
		" NEW ZIP " << calcInzipName(buffer_path));

	string new_emb_file = temp_path_ + '/' + new_inzip_name;
	FileName new_emb(new_emb_file);
	
	// need to make directory?
	FileName path = new_emb.onlyPath();
	if (!path.createPath()) {
		throw ExceptionMessage(ErrorException, _("Sync file failure"),
			bformat(_("Cannot create file path '%1$s'.\n"
			"Please check whether the path is writeable."),
			from_utf8(path.absFilename())));
		return;
	}

	if (old_emb.copyTo(new_emb)) {
		LYXERR(Debug::FILES, "Sync inzip file from " << inzip_name_ 
			<< " to " << new_inzip_name);
		inzip_name_ = new_inzip_name;
		return;
	}
	throw ExceptionMessage(ErrorException, _("Sync file failure"),
		 bformat(_("Cannot copy file %1$s to %2$s.\n"
				 "Please check whether the directory exists and is writeable."),
				from_utf8(old_emb_file), from_utf8(new_emb_file)));
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


void EmbeddedFileList::enable(bool flag, Buffer & buffer, bool updateFile)
{
	// update embedded file list
	update(buffer);
	
	int count_embedded = 0;
	int count_external = 0;
	iterator it = begin();
	iterator it_end = end();
	// an exception may be thrown
	for (; it != it_end; ++it) {
		it->enable(flag, &buffer, updateFile);
		if (it->embedded())
			++count_embedded;
		else
			++count_external;
	}
	// if operation is successful (no exception is thrown)
	buffer.params().embedded = flag;

	// if the operation is successful, update insets
	for (it = begin(); it != it_end; ++it)
		it->updateInsets();

	if (!updateFile || (count_external == 0 && count_embedded == 0))
		return;

	// show result
	if (flag) {
		docstring const msg = bformat(_("%1$d external files are ignored.\n"
			"%2$d embeddable files are embedded.\n"), count_external, count_embedded);
		Alert::information(_("Packing all files"), msg);
	} else {
		docstring const msg = bformat(_("%1$d external files are ignored.\n"
			"%2$d embedded files are extracted.\n"), count_external, count_embedded);
		Alert::information(_("Unpacking all files"), msg);
	}
}


void EmbeddedFileList::registerFile(EmbeddedFile const & file,
	Inset const * inset, Buffer const & buffer)
{
	BOOST_ASSERT(!buffer.embedded() || file.enabled());

	string newfile = file.absFilename();
	EmbeddedFile * efp = findFile(newfile);
	if (efp) {
		if (efp->embedded() != file.embedded()) {
			Alert::error(_("Wrong embedding status."),
				bformat(_("File %1$s is included in more than one insets, "
					"but with different embedding status. Assuming embedding status."),
					from_utf8(efp->outputFilename())));
			efp->setEmbed(true);
			// update the inset with this embedding status.
			const_cast<Inset*>(inset)->updateEmbeddedFile(*efp);
		}
		efp->addInset(inset);
		return;
	}
	file.clearInsets();
	push_back(file);
	back().addInset(inset);
}


EmbeddedFile const * EmbeddedFileList::findFile(std::string const & filename) const
{
	// try to find this file from the list
	std::vector<EmbeddedFile>::const_iterator it = begin();
	std::vector<EmbeddedFile>::const_iterator it_end = end();
	for (; it != it_end; ++it)
		if (it->absFilename() == filename)
			return &*it;
	return 0;
}


EmbeddedFile * EmbeddedFileList::findFile(std::string const & filename)
{
	// try to find this file from the list
	std::vector<EmbeddedFile>::iterator it = begin();
	std::vector<EmbeddedFile>::iterator it_end = end();
	for (; it != it_end; ++it)
		if (it->absFilename() == filename)
			return &*it;
	return 0;
}


void EmbeddedFileList::validate(Buffer const & buffer)
{
	clear();
	
	for (InsetIterator it = inset_iterator_begin(buffer.inset()); it; ++it)
		it->registerEmbeddedFiles(*this);

	iterator it = begin();
	iterator it_end = end();
	for (; it != it_end; ++it) {
		if (buffer.embedded() && it->embedded())
			// An exception will be raised if inzip file does not exist
			it->syncInzipFile(buffer.filePath());
		else
			// inzipName may be OS dependent
			it->setInzipName(it->calcInzipName(buffer.filePath()));
	}
	for (it = begin(); it != it_end; ++it)
		it->updateInsets();
	
	if (!buffer.embedded())
		return;

	// check if extra embedded files exist
	vector<string> extra = buffer.params().extraEmbeddedFiles();
	vector<string>::iterator e_it = extra.begin();
	vector<string>::iterator e_end = extra.end();
	for (; e_it != e_end; ++e_it) {
		EmbeddedFile file = EmbeddedFile(*e_it, buffer.filePath());
		// do not update from external file
		file.enable(true, &buffer, false);
		// but we do need to check file existence.
		if (!FileName(file.embeddedFile()).exists())
			throw ExceptionMessage(ErrorException, _("Failed to open file"),
				bformat(_("Embedded file %1$s does not exist. Did you tamper lyx temporary directory?"),
					file.displayName()));
	}
}


void EmbeddedFileList::update(Buffer const & buffer)
{
	clear();

	for (InsetIterator it = inset_iterator_begin(buffer.inset()); it; ++it)
		it->registerEmbeddedFiles(*this);

	// add extra embedded files
	vector<string> extra = buffer.params().extraEmbeddedFiles();
	vector<string>::iterator it = extra.begin();
	vector<string>::iterator it_end = extra.end();
	for (; it != it_end; ++it) {
		EmbeddedFile file = EmbeddedFile(*it, buffer.filePath());
		file.setEmbed(true);
		file.enable(buffer.embedded(), &buffer, false);
		insert(end(), file);
	}
}


bool EmbeddedFileList::writeFile(DocFileName const & filename, Buffer const & buffer)
{
	// file in the temporary path has the content
	string const content = FileName(addName(buffer.temppath(),
		"content.lyx")).toFilesystemEncoding();

	vector<pair<string, string> > filenames;
	// add content.lyx to filenames
	filenames.push_back(make_pair(content, "content.lyx"));
	// prepare list of embedded file
	update(buffer);
	//
	iterator it = begin();
	iterator it_end = end();
	for (; it != it_end; ++it) {
		if (it->embedded()) {
			string file = it->embeddedFile();
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
	DocFileName zipfile(addName(buffer.temppath(),
		onlyFilename(changeExtension(
			filename.toFilesystemEncoding(), ".zip"))));

	::zipFiles(zipfile.toFilesystemEncoding(), filenames);
	// copy file back
	if (!zipfile.copyTo(filename)) {
		Alert::error(_("Save failure"),
			 bformat(_("Cannot create file %1$s.\n"
					   "Please check whether the directory exists and is writeable."),
					 from_utf8(filename.absFilename())));
	}
	return true;
}

} // namespace lyx
