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

This file, and the embed checkbox in dialogs of InsetGraphics etc, implements
an 'Embedded Files' feature of lyx.


Expected features:
=========================

1. Bundled .lyx file can embed graphics, listings, bib file etc. The bundle
format is used when Document->Save in bundled format is selected.

2. Embedded file.lyx file is a zip file, with content.lyx and embedded files. 

3. The embedding status of embedded files are set from individual insets.

4. Extra files such as .cls and .layout can be embedded from Document->
Settings->Embedded Files->Extra Embedded Files.

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

Implementation:
======================

1. An EmbeddedFiles class is implemented to keep the embedded files (
class EmbeddedFile). (c.f. src/EmbeddedFiles.[h|cpp])

2. When a file is saved, it is scanned for embedded files. (c.f.
EmbeddedFiles::update(), Inset::registerEmbeddedFiles()).

3. When a lyx file file.lyx is saved, it is save to tmppath()/content.lyx
first. Embedded files are compressed along with content.lyx.
If embedding is disabled, file.lyx is saved in the usual pure-text format.
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
	///
	EmbeddedFile(std::string const & file = std::string(),
		std::string const & buffer_path = std::string());
	
	/// set filename and inzipName.
	/**
	 * NOTE: inzip_name_ is not unique across operation systems and is not 
	 * guaranteed to be the same across different versions of LyX.
	 * inzip_name_ will be saved to the LyX file, and is used to indicate 
	 * whether or not a file is embedded, and where the embedded file is in
	 * the bundled file. When a file is read, the stored inzip names are used
	 * at first. EmbeddedFiles::validate() will then scan these embedded files
	 * and update their inzip name, moving bundled files around if needed.
	 * This scheme has the advantage that it is safe to change how inzip files
	 * are saved in a bundled file.
	 *
	 * NOTE that this treatment does not welcome an UUID solution because
	 * all embedded files will have to be renamed when an embedded file is
	 * opened. It is of course possible to use saved inzipname, but that is
	 * not easy. For example, when a new EmbeddedFile is created with the same
	 * file as an old one, it needs to be synced to the old inzipname...
	**/
	void set(std::string const & filename, std::string const & buffer_path);
	/** Set the inzip name of an EmbeddedFile, which should be the name
	 *  of an actual embedded file on disk.
	 */
	void setInzipName(std::string const & name);

	/// filename in the zip file, which is related to buffer temp directory.
	std::string inzipName() const { return inzip_name_; }

	/// embedded file, equals to temppath()/inzipName()
	std::string embeddedFile() const;
	/// embeddedFile() or absFilename() depending on embedding status
	/// and whether or not embedding is enabled.
	FileName availableFile() const;
	/// relative file name or inzipName()
	std::string latexFilename(std::string const & buffer_path) const;

	/// add an inset that refers to this file
	void addInset(Inset const * inset);
	/// clear all isnets that associated with this file.
	void clearInsets() const { inset_list_.clear(); }

	/// embedding status of this file
	bool embedded() const { return embedded_; }
	/// set embedding status. 
	void setEmbed(bool embed);

	/// whether or not embedding is enabled for the current file
	/**
	 * An embedded file needs to know the temp path of a buffer to know
	 * where its embedded copy is. This has to be stored within EmbeddedFile
	 * because this class is often used when Buffer is unavailable. However,
	 * when an embedded file is copied to another buffer, temp_path_ has
	 * to be updated and file copying may be needed.
	 */
	bool isEnabled() const { return !temp_path_.empty(); }
	/// enable embedding of this file
	void enable(bool enabled, Buffer const & buf, bool updateFile);

	/// extract file, does not change embedding status
	bool extract() const;
	/// update embedded file from external file, does not change embedding status
	bool updateFromExternalFile() const;
	/// copy an embedded file to another buffer
	EmbeddedFile copyTo(Buffer const & buf);
	///
	/// After the embedding status is changed, update all insets related
	/// to this file item. For example, a graphic inset may need to monitor
	/// embedded file instead of external file.
	void updateInsets() const;

	/// Check readability of availableFile
	bool isReadableFile() const;
	/// Calculate checksum of availableFile
	unsigned long checksum() const;

	// calculate inzip_name_ from filename
	std::string calcInzipName(std::string const & buffer_path);
	// move an embedded disk file with an existing inzip_name_ to 
	// a calculated inzip_name_, if they differ.
	void syncInzipFile(std::string const & buffer_path);
private:
	/// filename in zip file
	std::string inzip_name_;
	/// the status of this docfile
	bool embedded_;
	/// Insets that contains this file item. Because a 
	/// file item can be referred by several Insets, a vector is used.
	mutable std::vector<Inset const *> inset_list_;
	/// Embedded file needs to know whether enbedding is enabled,
	/// and where is the lyx temporary directory. Such information can
	/// be retrived from a buffer, but a buffer is not always available when
	/// an EmbeddedFile is used.
	std::string temp_path_;
};


bool operator==(EmbeddedFile const & lhs, EmbeddedFile const & rhs);
bool operator!=(EmbeddedFile const & lhs, EmbeddedFile const & rhs);


class EmbeddedFileList : public std::vector<EmbeddedFile> {
public:
	/// set buffer params embedded flag. Files will be updated or extracted
	/// if such an operation fails, enable will fail.
	void enable(bool enabled, Buffer & buffer, bool updateFile);

	/// add a file item.
	/** \param file Embedded file to add
	 *  \param inset Inset pointer
	 */
	void registerFile(EmbeddedFile const & file, Inset const * inset,
		Buffer const & buffer);
	/// returns a pointer to the Embedded file representing this object,
	/// or null if not found. The filename should be absolute.
	const_iterator findFile(std::string const & filename) const;
	iterator findFile(std::string const & filename);

	/// validate embedded fies after a file is read.
	void validate(Buffer const & buffer);
	/// scan the buffer and get a list of EmbeddedFile
	void update(Buffer const & buffer);
	/// write a zip file
	bool writeFile(support::DocFileName const & filename, Buffer const & buffer);
};

} // namespace lyx

#endif
