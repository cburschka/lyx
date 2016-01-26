// -*- C++ -*-
/**
 * \file TempFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEMPFILE_H
#define TEMPFILE_H

#include <string>

namespace lyx {
namespace support {

class FileName;

/**
 * Class for safely creating temporary files without race conditions.
 * The file is created in the constructor, and deleted in the destructor.
 * You may do anything with the file (including deletion), but the instance
 * of this class must stay alive as long as the file is needed.
 * There is only one exception to this rule:
 * If the file is supposed to be used by a different process then you need
 * to be aware of OS specific file locking semantics: On windows, the file
 * is opened with exclusive rights for the process which opened it. This
 * is not the case on other OSes. Therefore, if the file is supposed to be
 * used by a different process you need to sometheing similar to TempName
 * in InsetExternal.cpp.
 */
class TempFile {
	/// noncopyable
	TempFile(TempFile const &);
	/// nonassignable
	TempFile & operator=(TempFile const &);
public:
	/**
	 *Create a temporary file with the given mask.
	 * \p mask must be in filesystem encoding, the template file
	 * will be created in the global temporary directory as given
	 * by 'package().temp_dir()'.
	 * If the mask contains "XXXXXX" this portion will be replaced by
	 * a uniquely generated string. If it does not contain this portion,
	 * it will be automatically appended using a dot. Therefore, please
	 * specify the "XXXXXX" portion if the extension of the generated
	 * name is important (e.g. for the converter machinery).
	 */
	TempFile(std::string const & mask);
	TempFile(FileName const & temp_dir, std::string const & mask);
	~TempFile();
	/**
	 * Get the name of the temporary file.
	 * This is empty if the file could not be created.
	 */
	FileName name() const;
	/**
	 * Set whether the file should be automatically deleted in the
	 * destructor.
	 * Automatic deletion is the default, but it can be switched off if
	 * the file should be kept, because it should be renamed afterwards.
	 */
	void setAutoRemove(bool autoremove);
private:
	///
	struct Private;
	Private * d;
};

} // namespace support
} // namespace lyx

#endif
