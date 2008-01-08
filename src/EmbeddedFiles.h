// -*- C++ -*-
/**
 * \file EmbeddedFiles.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#ifndef EMBEDDEDFILES_H
#define EMBEDDEDFILES_H

#include "support/FileName.h"

#include <string>
#include <vector>

/**

This file, and the embedding dialog implemented in src/frontends, implements
an 'Embedded Files' feature of lyx.


Expected features:
=========================

1. Bundled .lyx file can embed graphics, listings, bib file etc. The bundle
format is used when Document->Save in bundled format is selected.

2. Embedded file.lyx file is a zip file, with content.lyx and embedded files. 

3. The embedding status of embedded files can be set at the inset level,
or from Document->Settings->Embedded Files.

4. Extra files such as .cls and .layout can be embedded from Document->
Settings->Embedded Files->Extra Files.

5. When Document->Save in bundled format is selected, all embedded files
become bundled. Changes to the external version of this file does not
affect the output of the .lyx file.

6. When Document->Save in bundled format is unchecked, all embedded files
are copied to their original locations.

Overall, this feature allows two ways of editing a .lyx file

a. The continuous use of the pure-text .lyx file format with external
files. This is the default file format, and allows external editing
of .lyx file and better use of version control systems.

b. The embedded way. Figures etc are inserted to .lyx file and will
be embedded. These embedded files can be viewed or edited through
the embedding dialog. This file can be shared with others more
easily. 

Format a and b can be converted easily, by packing/unpacking a .lyx file.

NOTE: With current implementation, files with absolute filenames (not in
or deeper under the current document directory) can not be embedded.

Implementation:
======================

1. An EmbeddedFiles class is implemented to keep the embedded files (
class EmbeddedFile). (c.f. src/EmbeddedFiles.[h|cpp])

2. When a file is saved, it is scanned for embedded files. (c.f.
EmbeddedFiles::update(), Inset::registerEmbeddedFiles()).

3. When a lyx file file.lyx is saved, it is save to tmppath()/content.lyx
first. Embedded files are compressed along with content.lyx.
If embedding is disabled, file.lyx is saved in the usual pure-text form.
(c.f. Buffer::writeFile(), EmbeddedFiles::writeFile())

4. When a lyx file.lyx file is opened, if it is a zip file, it is
decompressed to tmppath() and tmppath()/content.lyx is read as usual.
(c.f. bool Buffer::readFile())

5. A menu item Document -> Save in bundled format is provided to pack/unpack
a .lyx file.

6. If embedding of a .lyx file with embedded files is disabled, all its
embedded files are copied to their respective external filenames. This
is why external filename will exist even if a file is at "EMBEDDED" status.

*/

namespace lyx {

class Buffer;
class Inset;
class Lexer;
class ErrorList;

class EmbeddedFile : public support::DocFileName
{
public:
	EmbeddedFile(std::string const & file = std::string(),
		std::string const & buffer_path = std::string());
	
	/// set filename and inzipName.
	void set(std::string const & filename, std::string const & buffer_path);

	/// filename in the zip file, which is the relative path
	std::string inzipName() const { return inzip_name_; }

	/// embedded file, equals to temppath()/inzipName()
	std::string embeddedFile() const;
	/// embeddedFile() or absFilename() depending on embedding status
	/// and whether or not embedding is enabled.
	FileName availableFile() const;
	/// 
	std::string latexFilename(std::string const & buffer_path) const;

	/// add an inset that refers to this file
	void addInset(Inset const * inset);
	int refCount() const { return inset_list_.size(); }

	/// embedding status of this file
	bool embedded() const { return embedded_; }
	/// set embedding status. updateFromExternal() should be called before this
	/// to copy or sync the embedded file with external one.
	void setEmbed(bool embed);

	/// whether or not embedding is enabled in the current buffer
	bool enabled() const { return temp_path_ != ""; }
	/// enable embedding of this file
	void enable(bool flag, Buffer const * buf);

	/// extract file, does not change embedding status
	bool extract() const;
	/// update embedded file from external file, does not change embedding status
	bool updateFromExternalFile() const;
	///
	/// After the embedding status is changed, update all insets related
	/// to this file item. For example, a graphic inset may need to monitor
	/// embedded file instead of external file. To make sure inset pointers 
	/// are up to date, please make sure there is no modification to the
	/// document between EmbeddedFiles::update() and this function.
	void updateInsets(Buffer const * buf) const;

	/// Check readability of availableFile
	bool isReadableFile() const;
	/// Calculate checksum of availableFile
	unsigned long checksum() const;

private:
	/// filename in zip file
	std::string inzip_name_;
	/// the status of this docfile
	bool embedded_;
	/// Insets that contains this file item. Because a 
	/// file item can be referred by several Insets, a vector is used.
	std::vector<Inset const *> inset_list_;
	/// Embedded file needs to know whether enbedding is enabled,
	/// and where is the lyx temporary directory. Such information can
	/// be retrived from a buffer, but a buffer is not always available when
	/// an EmbeddedFile is used.
	std::string temp_path_;
};


bool operator==(EmbeddedFile const & lhs, EmbeddedFile const & rhs);
bool operator!=(EmbeddedFile const & lhs, EmbeddedFile const & rhs);


class EmbeddedFiles {
public:
	typedef std::vector<EmbeddedFile> EmbeddedFileList;
public:
	///
	EmbeddedFiles(Buffer * buffer = 0) : file_list_(), buffer_(buffer) {}
	///
	~EmbeddedFiles() {}

	/// return buffer params embedded flag
	bool enabled() const;
	/// set buffer params embedded flag. Files will be updated or extracted
	/// if such an operation fails, enable will fail.
	void enable(bool flag);

	/// add a file item. 
	/* \param file Embedded file to add
	 * \param inset Inset pointer
	 */
	void registerFile(EmbeddedFile const & file, Inset const * inset = 0);

	/// scan the buffer and get a list of EmbeddedFile
	void update();

	/// write a zip file
	bool writeFile(support::DocFileName const & filename);

	void clear() { file_list_.clear(); }

	///
	EmbeddedFileList::iterator begin() { return file_list_.begin(); }
	EmbeddedFileList::iterator end() { return file_list_.end(); }
	EmbeddedFileList::const_iterator begin() const { return file_list_.begin(); }
	EmbeddedFileList::const_iterator end() const { return file_list_.end(); }

private:
	/// list of embedded files
	EmbeddedFileList file_list_;
	///
	Buffer * buffer_;
};

} // namespace lyx

#endif
