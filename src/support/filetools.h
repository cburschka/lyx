// -*- C++-*-
/**
 * \file filetools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 */

#ifndef LYX_FILETOOL_H
#define LYX_FILETOOL_H

#include <vector>
#include <utility>
#include "LString.h"


///
string const CreateBufferTmpDir(string const & pathfor = string());

/// Creates directory. Returns true on succes.
bool createDirectory(string const & name, int permissions);

///
string const CreateLyXTmpDir(string const & deflt);

///
int DestroyBufferTmpDir(string const & tmpdir);

///
int DestroyLyXTmpDir(string const & tmpdir);

/** Find file by searching several directories.
  Uses a string of paths separated by ";"s to find a file to open.
    Can't cope with pathnames with a ';' in them. Returns full path to file.
    If path entry begins with $$LyX/, use system_support.
    If path entry begins with $$User/, use user_support.
    Example: "$$User/doc;$$LyX/doc".
*/
string const FileOpenSearch(string const & path, string const & name,
		       string const & ext = string());

/** Returns the real name of file name in directory path, with optional
  extension ext.
  The file is searched in the given path (unless it is an absolute
  file name), first directly, and then with extension .ext (if given).
  */
string const FileSearch(string const & path, string const & name,
		  string const & ext = string());

/// Returns a vector of all files in directory dir having extension ext.
std::vector<string> const DirList(string const & dir,
				  string const & ext = string());

/** Is directory read only?
  returns
    true: dir writeable
    false: not writeable
*/
bool IsDirWriteable (string const & path);

/** Is a file readable ?
  Returns true if the file `path' is readable.
 */
bool IsFileReadable (string const & path);

/** Is file read only?
  returns
    1: read-write
    0: read_only
   -1: error (doesn't exist, no access, anything else)
  */
int IsFileWriteable (string const & path);

///
bool IsLyXFilename(string const & filename);

///
bool IsSGMLFilename(string const & filename);

/** Returns the path of a library data file.
  Search the file name.ext in the subdirectory dir of
  \begin{enumerate}
    \item user_support
    \item build_support (if not empty)
    \item system_support
  \end{enumerate}
    The third parameter `ext' is optional.
*/
string const LibFileSearch(string const & dir, string const & name,
		     string const & ext = string());

/** Same as LibFileSearch(), but tries first to find an
  internationalized version of the file by prepending $LANG_ to the
  name
  */
string const
i18nLibFileSearch(string const & dir, string const & name,
		  string const & ext = string());

/** Takes a command such as "sh $$s/scripts/convertDefault.sh file.in file.out"
 *  and replaces "$$s/" with the path to the LyX support directory containing
 *  this script. If the script is not found, "$$s/" is removed. Executing the
 *  command will still fail, but the error message will make some sort of
 *  sense ;-)
 */
std::string const LibScriptSearch(std::string const & command);

///
string const GetEnv(string const & envname);

/** Return the contents of the environment variable \c name,
 *  split using the OS-dependent token separating elements.
 *  Each element is then passed through os::internal_path to
 *  guarantee that it is in the form of a unix-stype path.
 *  If the environment variable is not set, then returns an empty vector.
 */
std::vector<string> const getEnvPath(string const & name);

/** Set the contents of the environment variable \c name
 *  using the paths stored in the \c env vector.
 *  Each element is passed through os::external_path.
 */
void setEnvPath(string const & name, std::vector<string> const & env);

/** Prepend a list of paths to that returned by the environment variable.
 *  Identical paths occurring later in the list are removed.
 *  @param name the name of the environment variable.
 *  @prefix the list of paths in OS-native syntax.
 *  Eg "/foo/bar:/usr/bin:/usr/local/bin" on *nix,
 *     "C:\foo\bar;C:\windows" on Windows.
 */
void prependEnvPath(string const & name, string const & prefix);

///
bool PutEnv(string const & envstr);

///
bool PutEnvPath(string const & envstr);

/** @param path a file path in internal_path format. Ie, directories
 *  are indicated by '/', not by '\'.
 *
 *  Manipulates @c path into a form suitable for inclusion in a LaTeX
 *  document.  
 *  If @c path contains LaTeX special characters, these are escaped.
 *  Eg, '~' -> '\string~'
 *  If @c path contains spaces, then the returned path is enclosed in
 *  "-quotes. This last fix will lead to successful compiliation of the
 *  LaTeX file only if a sufficiently modern LaTeX compiler is used.
 *  If @c exclude_extension is true the extension is left outside the quotes.
 *  This is needed for pdfeTeX, Version 3.141592-1.21a-2.2 (Web2C 7.5.4)
 *  (format=pdflatex 2005.4.11) in combination with
 *  pdftex.def 2002/06/19 v0.03k graphics/color for pdftex:
 *  It does not recognize the file extension if it is inside the quotes.
 */
string const latex_path(string const & path, bool exclude_extension = false);

/// Substitutes active latex characters with underscores in filename
string const MakeLatexName(string const & file);

/// Put the name in quotes suitable for the current shell
string const QuoteName(string const & file);

/// Add a filename to a path. Any path from filename is stripped first.
string const AddName(string const & path, string const & fname);

/// Append sub-directory(ies) to path in an intelligent way
string const AddPath(string const & path, string const & path2);

/** Change extension of oldname to extension.
 If oldname does not have an extension, it is appended.
 If the extension is empty, any extension is removed from the name.
 */
string const
ChangeExtension(string const & oldname, string const & extension);

/// Return the extension of the file (not including the .)
string const GetExtension(string const & name);

/// Return the type of the file as an extension from contents
string const getExtFromContents(string const & name);

/// check for zipped file
bool zippedFile(string const & name);

/// unzip a file
string const unzipFile(string const & zipped_file);

/// Returns true is path is absolute
bool AbsolutePath(string const & path);

/// Create absolute path. If impossible, don't do anything
string const ExpandPath(string const & path);

/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
string const MakeAbsPath(string const & RelPath = string(),
			 string const & BasePath = string());

/** Creates a nice compact path for displaying. The parameter
  threshold, if given, specifies the maximal length of the path.
  */
string const
MakeDisplayPath(string const & path, unsigned int threshold = 1000);

/** Makes relative path out of absolute path.
  If it is deeper than basepath,
  it's easy. If basepath and abspath share something (they are all deeper
  than some directory), it'll be rendered using ..'s. If they are completely
  different, then the absolute path will be used as relative path
  WARNING: the absolute path and base path must really be absolute paths!!!
  */
string const
MakeRelPath(string const & abspath, string const & basepath);

/// Strip filename from path name
string const OnlyPath(string const & fname);

/// Normalize a path. Constracts path/../path
string const NormalizePath(string const & path);

/// Strips path from filename
string const OnlyFilename(string const & fname);

/// Get the contents of a file as a huge string
string const GetFileContents(string const & fname);

/** Check and Replace Environmentvariables ${NAME} in Path.
    Replaces all occurences of these, if they are found in the
    environment.
    Variables are defined by Var := '${' [a-zA-Z_][a-zA-Z_0-9]* '}'
*/
string const ReplaceEnvironmentPath(string const & path);

/* Set \c link to the path \c file points to as a symbolic link.
   If \c resolve is true, then \c link is an absolute path
   Returns true if successful */
bool LyXReadLink(string const & file, string & link, bool resolve = false);

/// Uses kpsewhich to find tex files
string const findtexfile(string const & fil, string const & format);

/// remove the autosave-file and give a Message if it can't be done
void removeAutosaveFile(string const & filename);

/// read the BoundingBox entry from a ps/eps/pdf-file
string const readBB_from_PSFile(string const & file);

typedef std::pair<int, string> cmd_ret;

cmd_ret const RunCommand(string const & cmd);

#endif
