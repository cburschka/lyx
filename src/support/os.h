// -*- C++ -*-
/**
 * \file os.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 *
 * wrap OS-specific stuff
 */

#ifndef OS_H
#define OS_H

#include "support/strfwd.h"
#include <cstddef>


namespace lyx {
namespace support {
namespace os {

enum shell_type {
	UNIX,	// Do we have to distinguish sh and csh?
	CMD_EXE
};

enum path_case {
	CASE_UNCHANGED,
	CASE_ADJUSTED
};

enum file_access {
	EXISTING,
	CREATE
};

/// Do some work just once.
void init(int argc, char * argv[]);

/// Returns the i-th program argument in utf8 encoding.
std::string utf8_argv(int i);

/// Removes from the internal copy \p num program arguments starting from \p i.
void remove_internal_args(int i, int num);

/// Returns the name of the NULL device (/dev/null, null).
std::string const & nulldev();

/// Returns "/" on *nix, "C:/", etc on Windows.
std::string current_root();

///
shell_type shell();

/// Returns no. of minutes allowed for a command to complete.
int timeout_min();

/// Name of the python interpreter
/// @param reset True if the python path should be recomputed
std::string const python(bool reset = false);

///
bool isFilesystemCaseSensitive();

/// Extract the path common to both @c p1 and @c p2. DBCS aware!
/// \p p1 and \p p2 are encoded in ucs4, \returns the index to the end of
/// the last matching path component (the index may be pointing after the
/// end of @c p1 or @c p2 if their last char is not the path separator).
std::size_t common_path(docstring const & p1, docstring const & p2);

/// Converts a unix style path to host OS style.
/// \p p and the return value are encoded in utf8.
std::string external_path(std::string const & p);

/// Converts a host OS style path to unix style.
/// \p p and the return value are encoded in utf8.
std::string internal_path(std::string const & p);

/// Converts a host OS style path to a unicode safe unix style.
/// On Windows, this is achieved by using the short form of the path,
/// which can be safely passed to standard I/O functions expecting narrow
/// char paths even when the path contains non-ascii chars.
/// As the short form is only available for existing files, if the file is
/// to be accessed for writing, \param how should be set to CREATE.
/// \p p and the return value are encoded in utf8.
std::string safe_internal_path(std::string const & p, file_access how = EXISTING);

/// Converts a unix style path list to host OS style.
/// \p p and the return value are encoded in utf8.
std::string external_path_list(std::string const & p);

/// Converts a host OS style path list to unix style.
/// \p p and the return value are encoded in utf8.
std::string internal_path_list(std::string const & p);

/**
 * Converts a unix style path into a form suitable for inclusion in a LaTeX
 * document.
 * \p p is encoded in utf8.
 * Caution: This function handles only the OS specific part of that task.
 * Never use it directly, use lyx::support::latex_path instead.
 */
std::string latex_path(std::string const & p);

/**
 * Converts a platform style path list into a form suitable for the TeX engine.
 * \p p is encoded in utf8.
 */
std::string latex_path_list(std::string const & p);

/// Checks if the format string is suitable on the OS
bool is_valid_strftime(std::string const & p);

/** Returns a string suitable to be passed to popen when
 *  reading a file.
 */
char const * popen_read_mode();

enum path_type {
	PLATFORM,
	TEXENGINE
};

/** The character used to separate paths for platform environment variables
 *  (such as PATH) or for the TeX engine.
 */
char path_separator(path_type type = PLATFORM);

/** If @c use_windows_paths is true, LyX will output Windows-style paths to
 *  latex files rather than posix ones. Obviously, this option is used only
 *  under Windows.
 */
void windows_style_tex_paths(bool use_windows_paths);

enum auto_open_mode {
	VIEW,
	EDIT
};

/** Check whether or not a file can be opened by a default viewer or editor.
 *  \param extension (without leading .)
 *  \param mode can be opened in VIEW or EDIT mode
 *  \returns whether or not the format can be opened according to \p mode
 */
bool canAutoOpenFile(std::string const & ext, auto_open_mode const mode);

/** View or edit a file with the default viewer or editor.
 *  \param filename file to open (encoded in utf8)
 *  \param mode open in VIEW or EDIT mode
 *  \returns whether or not the file is viewed (or edited) successfully.
 */
bool autoOpenFile(std::string const & filename, auto_open_mode const mode,
		  std::string const & path = empty_string());

/** Resolves a path such that it does not contain '.', '..', or symbolic links.
  * \p path and the return value are encoded in utf8.
  */
std::string real_path(std::string const & path);

/** Checks whether \param path starts with \param pre, accounting for case
  * insensitive file systems.
  */
bool path_prefix_is(std::string const & path, std::string const & pre);

/** Checks whether \param path starts with \param pre, accounting for case
  * insensitive file systems. If true, the file system is case insensitive,
  * and \param how == CASE_ADJUSTED, the case of the matching prefix in
  * @c path is made equal to that of @c pre.
  */
bool path_prefix_is(std::string & path, std::string const & pre, path_case how = CASE_UNCHANGED);

} // namespace os
} // namespace support
} // namespace lyx

#endif
