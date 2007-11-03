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

Format a anb b can be converted easily, by packing/unpacking a .lyx file.

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
	EmbeddedFile() {};

	EmbeddedFile(std::string const & file, std::string const & inzip_name,
		bool embedded, Inset const * inset);

	/// filename in the zip file, usually the relative path
	std::string inzipName() const { return inzip_name_; }
	void setInzipName(std::string name) { inzip_name_ = name; }
	/// embedded file, equals to temppath()/inzipName()
	std::string embeddedFile(Buffer const * buf) const;
	/// embeddedFile() or absFilename() depending on embedding status
	std::string availableFile(Buffer const * buf) const;

	/// add an inset that refers to this file
	void addInset(Inset const * inset);
	Inset const * inset(int idx) const;
	/// save the location of this inset as bookmark so that
	/// it can be located using LFUN_BOOKMARK_GOTO
	void saveBookmark(Buffer const * buf, int idx) const;
	/// Number of Insets this file item is referred
	/// If refCount() == 0, this file must be manually inserted.
	/// This fact is used by the update() function to skip updating
	/// such items.
	int refCount() const { return inset_list_.size(); }

	/// embedding status of this file
	bool embedded() const { return embedded_; }
	/// set embedding status. updateFromExternal() should be called before this
	/// to copy or sync the embedded file with external one.
	void setEmbed(bool embed) { embedded_ = embed; }

	/// extract file, does not change embedding status
	bool extract(Buffer const * buf) const;
	/// update embedded file from external file, does not change embedding status
	bool updateFromExternalFile(Buffer const * buf) const;
	///
	/// After the embedding status is changed, update all insets related
	/// to this file item.
	/// Because inset pointers may not be up to date, EmbeddedFiles::update()
	/// would better be called before this function is called.
	void updateInsets(Buffer const * buf) const;

private:
	/// filename in zip file
	std::string inzip_name_;
	/// the status of this docfile
	bool embedded_;
	/// Insets that contains this file item. Because a 
	/// file item can be referred by several Insets, a vector is used.
	std::vector<Inset const *> inset_list_;
};


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
	/* \param filename filename to add
	 * \param embed embedding status. For a new file item, this is always true.
	 *    If the file already exists, this parameter is ignored.
	 * \param inset Inset pointer
	 * \param inzipName suggested inzipname
	 */
	EmbeddedFile & registerFile(std::string const & filename, bool embed = false,
		Inset const * inset = 0,
		std::string const & inzipName = std::string());

	/// scan the buffer and get a list of EmbeddedFile
	void update();

	/// write a zip file
	bool writeFile(support::DocFileName const & filename);

	void clear() { file_list_.clear(); }

	///
	EmbeddedFile & operator[](size_t idx) { return *(file_list_.begin() + idx); }
	EmbeddedFile const & operator[](size_t idx) const { return *(file_list_.begin() + idx); }
	///
	EmbeddedFileList::iterator begin() { return file_list_.begin(); }
	EmbeddedFileList::iterator end() { return file_list_.end(); }
	EmbeddedFileList::const_iterator begin() const { return file_list_.begin(); }
	EmbeddedFileList::const_iterator end() const { return file_list_.end(); }
	// try to locate filename, using either absFilename() or embeddedFile()
	EmbeddedFileList::const_iterator find(std::string filename) const;
	/// extract all file items, used when disable embedding
	bool extract() const;
	/// update all files from external, used when enable embedding
	bool updateFromExternalFile() const;
	///
	/// update all insets to use embedded files when embedding status is changed
	void updateInsets() const;
private:
	/// get a unique inzip name, a suggestion can be given.
	std::string const getInzipName(std::string const & name, std::string const & inzipName);
	/// list of embedded files
	EmbeddedFileList file_list_;
	///
	Buffer * buffer_;
};

} // namespace lyx

#endif
