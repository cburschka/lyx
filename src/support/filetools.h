// -*- C++ -*-
/**
 * \file filetools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_FILETOOL_H
#define LYX_FILETOOL_H

#include "support/docstring.h"

#include <utility>
#include <string>

namespace lyx {
namespace support {

class FileName;

/** Creates the global LyX temp dir.
  \p deflt can be an existing directory name. In this case a new directory
  inside \p deflt is created. If \p deflt does not exist yet, \p deflt is
  created and used as the temporary directory.
  \return the tmp dir name or string() if something went wrong.
 */
FileName const createLyXTmpDir(FileName const & deflt);

#if 0
// FIXME unused. Should this be deleted or resurrected?
/** Find file by searching several directories.
  Uses a string of paths separated by ";"s to find a file to open.
    Can't cope with pathnames with a ';' in them. Returns full path to file.
    If path entry begins with $$LyX/, use system_lyxdir.
    If path entry begins with $$User/, use user_lyxdir.
    Example: "$$User/doc;$$LyX/doc".
*/
FileName const fileOpenSearch(std::string const & path,
				 std::string const & name,
				 std::string const & ext = std::string());
#endif

/// How to search files
enum search_mode {
	/// The file must exist (return an empty file name otherwise)
	must_exist,
	/// Only do file name expansion, return the complete name even if
	/// the file does not exist
	may_not_exist
};

/** Returns the real name of file name in directory path, with optional
  extension ext.
  The file is searched in the given path (unless it is an absolute
  file name), first directly, and then with extension .ext (if given).
  */
FileName const fileSearch(std::string const & path,
			     std::string const & name,
			     std::string const & ext = std::string(),
			     search_mode mode = must_exist);

///
bool isLyXFileName(std::string const & filename);

///
bool isSGMLFileName(std::string const & filename);

/// check for characters in filenames not allowed by LaTeX
bool isValidLaTeXFileName(std::string const & filename);

/** check for characters in filenames that might lead to
  problems when manually compiling the LaTeX export of LyX
  and opening the result with some older DVI-viewers
*/
bool isValidDVIFileName(std::string const & filename);

/// check whether the file has binary contents
bool isBinaryFile(FileName const & filename);

/** Returns the path of a library data file.
    Search the file name.ext in the subdirectory dir of
      -# user_lyxdir
      -# build_lyxdir (if not empty)
      -# system_lyxdir
    The third parameter `ext' is optional.
*/
FileName const libFileSearch(std::string const & dir,
				std::string const & name,
				std::string const & ext = std::string());

/** Same as libFileSearch(), but tries first to find an
  internationalized version of the file by prepending $LANG_ to the
  name
  */
FileName const
i18nLibFileSearch(std::string const & dir,
		  std::string const & name,
		  std::string const & ext = std::string());

/** Same as libFileSearch(), but tries first to find a preferred
  version of the file by appending the icon set name to the dir
  */
FileName const
imageLibFileSearch(std::string & dir, std::string const & name,
		   std::string const & ext = std::string());

/// How to quote a filename
enum quote_style {
	/** Quote for the (OS dependant) shell. This is needed for command
	    line arguments of subprocesses. */
	quote_shell,
	/** Quote for python. Use this if you want to store a filename in a
	    python script. Example: \code
	    os << "infile = " << quoteName(filename) << '\\n';
	    \endcode This uses double quotes, so that you can also use this
	    to quote filenames as part of a string if the string is quoted
	    with single quotes. */
	quote_python
};

/** Takes a command such as "python $$s/scripts/convertDefault.py file.in file.out"
 *  and replaces "$$s/" with the path to the LyX support directory containing
 *  this script. If the script is not found, "$$s/" is removed. Executing the
 *  command will still fail, but the error message will make some sort of
 *  sense ;-)
 */
std::string const commandPrep(std::string const & command);

enum latex_path_extension {
	PROTECT_EXTENSION,
	EXCLUDE_EXTENSION
};

enum latex_path_dots {
	LEAVE_DOTS,
	ESCAPE_DOTS
};

/** @param path a file path in internal_path format. Ie, directories
 *  are indicated by '/', not by '\'.
 *
 *  Manipulates @c path into a form suitable for inclusion in a LaTeX
 *  document.
 *  If @c path contains LaTeX special characters, these are escaped.
 *  Eg, '~' -> '\\string~'
 *  If @c path contains spaces, then the returned path is enclosed in
 *  "-quotes. This last fix will lead to successful compiliation of the
 *  LaTeX file only if a sufficiently modern LaTeX compiler is used.
 *  If @c ext == EXCLUDE_EXTENSION the extension is left outside the quotes.
 *  This is needed for pdfeTeX, Version 3.141592-1.21a-2.2 (Web2C 7.5.4)
 *  (format=pdflatex 2005.4.11) in combination with
 *  pdftex.def 2002/06/19 v0.03k graphics/color for pdftex:
 *  It does not recognize the file extension if it is inside the quotes.
 *  If @c dots == ESCAPE_DOTS dots in the filename are replaced by
 *  "\\lyxdot ". This is needed for the \\includegraphics command if the
 *  automatic format selection is used.
 */
std::string const latex_path(std::string const & path,
		latex_path_extension extension = PROTECT_EXTENSION,
		latex_path_dots dots = LEAVE_DOTS);

/// Substitutes active latex characters with underscores in filename
FileName const makeLatexName(FileName const & file);

/** Put the name in quotes suitable for the current shell or python,
    depending on \p style. */
std::string const quoteName(std::string const & file, quote_style style = quote_shell);

/// Add a filename to a path. Any path from filename is stripped first.
std::string const addName(std::string const & path, std::string const & fname);

/// Append sub-directory(ies) to path in an intelligent way
std::string const addPath(std::string const & path, std::string const & path2);

/** Change extension of oldname to extension.
 If oldname does not have an extension, it is appended.
 If the extension is empty, any extension is removed from the name.
 */
std::string const
changeExtension(std::string const & oldname, std::string const & extension);

/// Remove the extension from \p name
std::string const removeExtension(std::string const & name);

/** Add the extension \p ext to \p name.
 Use this instead of changeExtension if you know that \p name is without
 extension, because changeExtension would wrongly interpret \p name if it
 contains a dot.
 */
std::string const
addExtension(std::string const & name, std::string const & extension);

/// Return the extension of the file (not including the .)
std::string const getExtension(std::string const & name);

/** \return the name that LyX will give to the unzipped file \p zipped_file
  if the second argument of unzipFile() is empty.
 */
std::string const unzippedFileName(std::string const & zipped_file);

/** Unzip \p zipped_file.
  The unzipped file is named \p unzipped_file if \p unzipped_file is not
  empty, and unzippedFileName(\p zipped_file) otherwise.
  Will overwrite an already existing unzipped file without warning.
 */
FileName const unzipFile(FileName const & zipped_file,
			 std::string const & unzipped_file = std::string());

/// Create absolute path. If impossible, don't do anything
std::string const expandPath(std::string const & path);

/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
FileName const makeAbsPath(std::string const & RelPath = std::string(),
			      std::string const & BasePath = std::string());

/** Creates a nice compact path for displaying. The parameter
  threshold, if given, specifies the maximal length of the path.
  */
docstring const
makeDisplayPath(std::string const & path, unsigned int threshold = 1000);

/** Makes relative path out of absolute path.
  If it is deeper than basepath,
  it's easy. If basepath and abspath share something (they are all deeper
  than some directory), it'll be rendered using ..'s. If they are completely
  different, then the absolute path will be used as relative path
  WARNING: the absolute path and base path must really be absolute paths!!!
  */
docstring const
makeRelPath(docstring const & abspath, docstring const & basepath);

/// Strip filename from path name
std::string const onlyPath(std::string const & fname);

/// Strips path from filename
std::string const onlyFileName(std::string const & fname);

/** Check and Replace Environmentvariables ${NAME} in Path.
    Replaces all occurences of these, if they are found in the
    environment.
    Variables are defined by Var := '${' [a-zA-Z_][a-zA-Z_0-9]* '}'
*/
std::string const replaceEnvironmentPath(std::string const & path);

/**
   Return a string to be used as a prefix to a command for setting the
   environment of the TeX engine with respect to the path \p path.
 */
std::string latexEnvCmdPrefix(std::string const & path);

/** Replace all references to a current directory (a lonely '.' or
    the prefix "./") in \c pathlist with \c path. Also prefixes
    all non-absolute paths with \c path.
*/
std::string const replaceCurdirPath(std::string const & path, std::string const & pathlist);

/** Set \c link to the path \c file points to as a symbolic link.
    \return true if successful.
 */
bool readLink(FileName const & file, FileName & link);

/**
 * Search a TeX file in all locations the latex compiler would search it,
 * with the help of kpsewhich.
 * The current working directory must be set correctly, so that relative
 * names work.
 * \param fil The filename to search
 * \param format The file format as used by kpsewhich, e.g. "bib", "bst" etc.
 */
FileName const findtexfile(std::string const & fil,
			      std::string const & format);

/** \param file1, file2 the two files to be compared. Must have absolute paths.
 *  \returns 1 if \c file1 has a more recent timestamp than \c file2,
 *           0 if their timestamps are the same,
 *          -1 if \c file2 has a more recent timestamp than \c file1.
 *  If one of the files does not exist, the return value indicates the file
 *  which does exist. Eg, if \c file1 exists but \c file2 does not, return 1.
 */
int compare_timestamps(FileName const & file1, FileName const & file2);

/// \param lfuns: true if we're converting lfuns, false if prefs
bool prefs2prefs(FileName const & filename, FileName const & tempfile,
                 bool lfuns);

typedef std::pair<int, std::string> cmd_ret;

cmd_ret const runCommand(std::string const & cmd);

int fileLock(const char * lock_file);
void fileUnlock(int fd, const char * lock_file);

} // namespace support
} // namespace lyx



#endif
