// -*- C++ -*-
/**
 * \file FileName.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILENAME_H
#define FILENAME_H

#include "support/strfwd.h"

#include <ctime>


namespace lyx {
namespace support {

/// Defined in "FileNameList.h".
class FileNameList;

/**
 * Class for storing file names.
 * The file name may be empty. If it is not empty it is an absolute path.
 * The file may or may not exist.
 */
class FileName {
public:
	/// Constructor for empty filenames
	FileName();
	/** Constructor for nonempty filenames.
	 * explicit because we don't want implicit conversion of relative
	 * paths in function arguments (e.g. of unlink).
	 * \param abs_filename the file in question. Must have an absolute path.
	 * Encoding is always UTF-8.
	 */
	explicit FileName(std::string const & abs_filename);

	/// copy constructor.
	FileName(FileName const &);

	/// constructor with base name and suffix.
	FileName(FileName const & fn, std::string const & suffix);

	///
	FileName & operator=(FileName const &);

	virtual ~FileName();
	/** Set a new filename.
	 * \param filename the file in question. Must have an absolute path.
	 * Encoding is always UTF-8.
	 */
	virtual void set(std::string const & filename);
	virtual void set(FileName const & fn, std::string const & suffix);
	virtual void erase();
	/// Is this filename empty?
	bool empty() const;
	/// Is the filename absolute?
	bool isAbsolute() const;

	/// get the absolute file name in UTF-8 encoding
	std::string absFilename() const;
	/**
	 * Get the file name in the encoding used by the file system.
	 * Only use this for accessing the file, e.g. with an fstream.
	 */
	std::string toFilesystemEncoding() const;

	/// returns true if the file exists
	bool exists() const;
	/// \return true if this object points to a symbolic link.
	bool isSymLink() const;
	/// \return true if the file is empty.
	bool isFileEmpty() const;
	/// returns time of last write access
	std::time_t lastModified() const;
	/// generates a checksum of a file
	virtual unsigned long checksum() const;
	/// return true when file is readable but not writabel
	bool isReadOnly() const;
	/// return true when it names a directory
	bool isDirectory() const;
	/// return true when directory is readable
	bool isReadableDirectory() const;
	/// return true when it is a file and readable
	virtual bool isReadableFile() const;
	/// return true when file/directory is writable
	bool isWritable() const;
	/// return true when file/directory is writable (write test file)
	bool isDirWritable() const;
	/// \return list other files in the directory having optional extension 'ext'.
	FileNameList dirList(std::string const & ext) const;
	
	/// copy a file
	/// \return true when file/directory is writable (write test file)
	/// \warning This methods has different semantics when system level
	/// copy command, it will overwrite the \c target file if it exists,
	bool copyTo(FileName const & target) const;

	/// remove pointed file.
	/// \return true on success.
	bool removeFile() const;

	/// rename pointed file.
	/// \return false if the operation fails or if the \param target file
	/// already exists.
	/// \return true on success.
	bool renameTo(FileName const & target) const;

	/// move pointed file to \param target.
	/// \return true on success.
	bool moveTo(FileName const & target) const;

	/// change mode of pointed file.
	/// This methods does nothing and return true on platforms that does not
	/// support this.
	/// \return true on success.
	bool changePermission(unsigned long int mode) const;

	/// remove pointed directory and all contents.
	/// \return true on success.
	bool destroyDirectory() const;
	/// Creates pointed directory.
	/// \return true on success.
	bool createDirectory(int permissions) const;
	/// Creates pointed path.
	/// \return true on success.
	bool createPath() const;

	/// Get the contents of a file as a huge docstring.
	/// \param encoding defines the encoding of the file contents.
	/// Only four encodings are supported:
	/// "UTF-8", "ascii", "latin1" and "local8bit" which uses the
	/// current system locale.
	docstring fileContents(std::string const & encoding) const;

	/// Change extension.
	/**
	* If oldname does not have an extension, it is appended.
	* If the extension is empty, any extension is removed from the name.
	*/
	void changeExtension(std::string const & extension);

	/** Guess the file format name (as in Format::name()) from contents.
	 Normally you don't want to use this directly, but rather
	 Formats::getFormatFromFile().
	 */
	std::string guessFormatFromContents() const;

	/// check for zipped file
	bool isZippedFile() const;

	static FileName fromFilesystemEncoding(std::string const & name);
	/// (securely) create a temporary file with the given mask.
	/// \p mask must be in filesystem encoding, if it contains a
	/// relative path, the template file will be created in the global
	/// temporary directory as given by 'package().temp_dir()'.
	static FileName tempName(std::string const & mask = empty_string());
	static FileName tempName(FileName const & temp_dir,
		std::string const & mask);

	/// get the current working directory
	static FileName getcwd();

	static FileName tempPath();

	/// filename without path
	std::string onlyFileName() const;
	/// filename without path and without extension
	std::string onlyFileNameWithoutExt() const;
	/// only extension after the last dot.
	std::string extension() const;
	/// path without file name
	FileName onlyPath() const;
	/// used for display in the Gui
	docstring displayName(int threshold = 1000) const;

	/// change to a directory, return success
	bool chdir() const;
	
	/// \param buffer_path if empty, uses `pwd`
	docstring const relPath(std::string const & path) const;
	
	docstring const absoluteFilePath() const;

private:
	friend bool operator==(FileName const &, FileName const &);
	///
	struct Private;
	Private * const d;
};


bool operator==(FileName const &, FileName const &);
bool operator!=(FileName const &, FileName const &);
bool operator<(FileName const &, FileName const &);
bool operator>(FileName const &, FileName const &);
std::ostream & operator<<(std::ostream &, FileName const &);


/**
 * Class for storing file names that appear in documents (e. g. child
 * documents, included figures etc).
 * The file name must not denote a file in our temporary directory, but a
 * file that the user chose.
 */
class DocFileName : public FileName {
public:
	DocFileName();
	/** \param abs_filename the file in question. Must have an absolute path.
	 *  \param save_abs_path how is the file to be output to file?
	 */
	DocFileName(std::string const & abs_filename, bool save_abs_path = true);
	DocFileName(FileName const & abs_filename, bool save_abs_path = true);

	/** \param filename the file in question. May have either a relative
	 *  or an absolute path.
	 *  \param buffer_path if \c filename has a relative path, generate
	 *  the absolute path using this.
	 */
	virtual void set(std::string const & filename, std::string const & buffer_path);

	void erase();

	bool saveAbsPath() const { return save_abs_path_; }
	/// \param buffer_path if empty, uses `pwd`
	std::string relFilename(std::string const & buffer_path = empty_string()) const;
	/// \param buf_path if empty, uses `pwd`
	std::string outputFilename(std::string const & buf_path = empty_string()) const;
	
	/** @returns a mangled representation of the absolute file name
	 *  suitable for use in the temp dir when, for example, converting
	 *  an image file to another format.
	 *
	 *  @param dir the directory that will contain this file with
	 *  its mangled name. This information is used by the mangling
	 *  algorithm when determining the maximum allowable length of
	 *  the mangled name.
	 *
	 *  An example of a mangled name:
	 *  C:/foo bar/baz.eps -> 0C__foo_bar_baz.eps
	 *
	 *  It is guaranteed that
	 *  - two different filenames have different mangled names
	 *  - two FileName instances with the same filename have identical
	 *    mangled names.
	 *
	 *  Only the mangled file name is returned. It is not prepended
	 *  with @c dir.
	 */
	std::string
	mangledFilename(std::string const & dir = empty_string()) const;

	/// \return true if the file is compressed.
	bool isZipped() const;
	/// \return the absolute file name without its .gz, .z, .Z extension
	std::string unzippedFilename() const;

private:
	bool save_abs_path_;
	/// Cache for isZipped() because zippedFile() is expensive
	mutable bool zipped_;
	/// Is zipped_ valid?
	mutable bool zipped_valid_;
};


bool operator==(DocFileName const &, DocFileName const &);
bool operator!=(DocFileName const &, DocFileName const &);

} // namespace support
} // namespace lyx

#endif
