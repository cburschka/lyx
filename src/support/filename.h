// -*- C++ -*-
/**
 * \file filename.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILENAME_H
#define FILENAME_H

#include "LString.h"


namespace lyx {
namespace support {


class FileName {
public:
	FileName();
	/** \param filename the file in question. Must have an absolute path.
	 *  \param save_abs_path how is the file to be output to file?
	 */
	FileName(string const & abs_filename, bool save_abs_path = true);

	/** \param filename the file in question. May have either a relative
	 *  or an absolute path.
	 *  \param buffer_path if \c filename has a relative path, generate
	 *  the absolute path using this.
	 */
	void set(string const & filename, string const & buffer_path);

	void erase();
	bool empty() const { return name_.empty(); }

	bool saveAbsPath() const { return save_abs_path_; }
	string const absFilename() const { return name_; }
	/// \param buffer_path if empty, uses `pwd`
	string const relFilename(string const & buffer_path = string()) const;
	/// \param buf_path if empty, uses `pwd`
	string const outputFilename(string const & buf_path = string()) const;
	/** \return a mangled version of the absolute file name,
	 *  suitable for use in the temp dir when, for example, converting
	 *  an image file to another format.
	 */
	string const mangledFilename() const;

	/// \return true if the file is compressed.
	bool isZipped() const;
	/// \return the absolute file name without its .gz, .z, .Z extension
	string const unzippedFilename() const;

private:
	string name_;
	bool save_abs_path_;
};


bool operator==(FileName const &, FileName const &);
bool operator!=(FileName const &, FileName const &);


} // namespace support
} // namespace lyx

#endif
