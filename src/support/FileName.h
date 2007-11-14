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

#include "strfwd.h"

#include <string>
#include <ctime>


namespace lyx {
namespace support {


/**
 * Class for storing file names.
 * The file name may be empty. If it is not empty it is an absolute path.
 * The file may or may not exist.
 */
class FileName {
public:
	/// Constructor for empty filenames
	FileName() {}
	/** Constructor for nonempty filenames.
	 * explicit because we don't want implicit conversion of relative
	 * paths in function arguments (e.g. of unlink).
	 * \param abs_filename the file in question. Must have an absolute path.
	 * Encoding is always UTF-8.
	 */
	explicit FileName(std::string const & abs_filename);
	virtual ~FileName() {}
	/** Set a new filename.
	 * \param filename the file in question. Must have an absolute path.
	 * Encoding is always UTF-8.
	 */
	virtual void set(std::string const & filename);
	virtual void erase();
	/// Is this filename empty?
	bool empty() const { return name_.empty(); }
	/// get the absolute file name in UTF-8 encoding
	std::string const absFilename() const { return name_; }
	/**
	 * Get the file name in the encoding used by the file system.
	 * Only use this for accessing the file, e.g. with an fstream.
	 */
	std::string toFilesystemEncoding() const;

	/// returns true if the file exists
	bool exists() const;
	/// returns time of last write access
	std::time_t lastModified() const;
	/// return true when file is readable but not writabel
	bool isReadOnly() const;
	/// return true when it names a directory
	bool isDirectory() const;
	/// return true when file/directory is readable
	bool isReadable() const;
	/// return true when it is a file and readable
	bool isFileReadable() const;
	/// return true when file/directory is writable
	bool isWritable() const;
	/// return true when file/directory is writable (write test file)
	bool isDirWritable() const;
	
	/// return true when file/directory is writable (write test file)
	bool copyTo(FileName const & target, bool noclobber) const;

	/// remove directory and all contents, returns true on success
	bool destroyDirectory() const;
	/// Creates directory. Returns true on success
	bool createDirectory(int permissions) const;

	/// Get the contents of a file as a huge std::string
	std::string fileContents() const;
	/**
	 * Get a FileName from \p name in the encoding used by the file system.
	 * Only use this for filenames you got directly from the file system,
	 * e.g. from reading a directory.
	 * \p name must have an absolute path.
	 */

	/** Guess the file format name (as in Format::name()) from contents.
	 Normally you don't want to use this directly, but rather
	 Formats::getFormatFromFile().
	 */
	std::string guessFormatFromContents() const;

	/// check for zipped file
	bool isZippedFile() const;

	static FileName fromFilesystemEncoding(std::string const & name);
	/// (securely) create a temporary file in the given dir with the given mask
	/// \p mask must be in filesystem encoding
	static FileName tempName(FileName const & dir = FileName(),
						std::string const & mask = std::string());

	/// filename without path
	std::string onlyFileName() const;
	/// path without file name
	std::string onlyPath() const;
	/// used for display in the Gui
	docstring displayName(int threshold = 1000) const;


protected:
	/// The absolute file name in UTF-8 encoding.
	std::string name_;
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
	void set(std::string const & filename, std::string const & buffer_path);

	void erase();

	bool saveAbsPath() const { return save_abs_path_; }
	/// \param buffer_path if empty, uses `pwd`
	std::string const relFilename(std::string const & buffer_path = std::string()) const;
	/// \param buf_path if empty, uses `pwd`
	std::string const outputFilename(std::string const & buf_path = std::string()) const;
	
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
	std::string const
	mangledFilename(std::string const & dir = std::string()) const;

	/// \return true if the file is compressed.
	bool isZipped() const;
	/// \return the absolute file name without its .gz, .z, .Z extension
	std::string const unzippedFilename() const;

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
