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
#include <utility>

#include "ParIterator.h"
#include "Paragraph.h"

/**

This file, and the embedding dialog implemented in src/frontends, implements
an 'Embedded Files' feature of lyx.


Expected features:
=========================

1. With embedding enabled (disabled by default), .lyx file can embed graphics,
listings, bib file etc.

2. Embedded file.lyx file is a zip file, with file.lyx, manifest.txt
and embedded files. 

3. An embedding dialog is provided to change embedding status (buffer
level or individual embedded files), manually embed, extract, view
or edit files.

Overall, this feature allows two ways of editing a .lyx file

a. The continuous use of the pure-text .lyx file format with external
files. This is the default file format, and allows external editing
of .lyx file and better use of version control systems.

b. The embedded way. Figures etc are inserted to .lyx file and will
be embedded. These embedded files can be viewed or edited through
the embedding dialog. This file can be shared with others more
easily. 

Format a anb b can be converted easily, by enable/disable embedding. Diabling
embedding is also called unpacking because all embedded files will be copied
to their original locations.

Implementation:
======================

1. An EmbeddedFiles class is implemented to keep the embedded files (
class EmbeddedFile). (c.f. src/EmbeddedFiles.[h|cpp])
This class keeps a manifest that has
  a. external relative filename
  b. inzip filename. It is the relative path name if the embedded file is
    in or under the document directory, or file name otherwise. Name aliasing
    is used if two external files share the same name.
  c. embedding status.
It also provides functions to
  a. manipulate manifest
  b. scan a buffer for embeddable files
  c. determine which file to use according to embedding status

2. When a file is saved, it is scanned for embedded files. (c.f.
EmbeddedFiles::update(), Inset::registerEmbeddedFiles()).

3. When a lyx file file.lyx is saved, it is save to tmppath() first.
Embedded files are compressed along with file.lyx and a manifest.txt. 
If embedding is disabled, file.lyx is saved in the usual pure-text form.
(c.f. Buffer::writeFile(), EmbeddedFiles::writeFile())

4. When a lyx file.lyx file is opened, if it is a zip file, it is
decompressed to tmppath(). If manifest.txt and file.lyx exists in
tmppath(), the manifest is read to buffer, and tmppath()/file.lyx is
read as usual. If file.lyx is not a zip file, it is read as usual.
(c.f. bool Buffer::readFile())

5. A menu item Document -> Embedded Files is provided to open
a embedding dialog. It handles a EmbddedFiles point directly.
From this dialog, a user can disable embedding, change embedding status,
or embed other files, extract, view, edit files.

6. If embedding of a .lyx file with embedded files is disabled, all its
embedded files are copied to their respective external filenames. This
is why external filename will exist even if a file is at "EMBEDDED" status.

7. Individual embeddable insets should find ways to handle embedded files.
InsetGraphics replace params().filename with its temppath()/inzipname version
when the inset is created. The filename appears as /tmp/..../inzipname
when lyx runs. When params().filename is saved, lyx checks if this is an
embedded file (check path == temppath()), if so, save filename() instead.
(c.f. InsetGraphic::read(), InsetGraphics::edit(), InsetGraphicsParams::write())


*/

namespace lyx {

class Buffer;
class Lexer;
class ErrorList;

class EmbeddedFile : public support::DocFileName
{
public:
	EmbeddedFile(std::string const & file, std::string const & inzip_name,
		bool embedded, Inset const * inset);

	/// filename in the zip file, usually the relative path
	std::string inzipName() const { return inzip_name_; }
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

	// A flag indicating whether or not this filename is valid.
	// When lyx runs, InsetGraphics etc may be added or removed so filename
	// maybe obsolete. In Buffer::updateEmbeddedFiles, the EmbeddedFiles is first
	// invalidated (c.f. invalidate()), and all insets are asked to register
	// embedded files. In this way, EmbeddedFileList will be refreshed, with
	// status setting untouched.
	bool valid() const { return valid_; }
	void validate() { valid_ = true; }
	void invalidate();

	/// extract file, does not change embedding status
	bool extract(Buffer const * buf) const;
	/// update embedded file from external file, does not change embedding status
	bool updateFromExternalFile(Buffer const * buf) const;

private:
	/// filename in zip file
	std::string inzip_name_;
	/// the status of this docfile
	bool embedded_;
	///
	bool valid_;
	/// Current position of the item, used to locate the files. Because one
	/// file item can be referred by several Insets, a vector is used.
	std::vector<Inset const *> inset_list_;
};


class EmbeddedFiles {
public:
	typedef std::vector<EmbeddedFile> EmbeddedFileList;
public:
	///
	EmbeddedFiles(Buffer * buffer = NULL): file_list_(), buffer_(buffer) {}
	///
	~EmbeddedFiles() {}

	/// return buffer params embedded flag
	bool enabled() const;
	/// set buffer params embedded flag. Files will be updated or extracted
	/// if such an operation fails, enable will fail.
	bool enable(bool flag);

	/// add a file item. 
	/* \param filename filename to add
	 * \param embed embedding status. For a new file item, this is always true.
	 *    If the file already exists, this parameter is ignored.
	 * \param inset Inset pointer
	 * \param inzipName suggested inzipname
	 */
	void registerFile(std::string const & filename, bool embed = false,
		Inset const * inset = NULL,
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
	bool readManifest(Lexer & lex, ErrorList & errorList);
	void writeManifest(std::ostream & os) const;
private:
	/// get a unique inzip name, a suggestion can be given.
	std::string const getInzipName(std::string const & name, std::string const & inzipName);
	/// list of embedded files
	EmbeddedFileList file_list_;
	///
	Buffer * buffer_;
};


}
#endif
