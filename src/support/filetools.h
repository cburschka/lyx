// -*- C++-*-
/* lyx-filetool.h : tools functions for file/path handling
   this file is part of LyX, the High Level Word Processor
   Copyright 1995-2000, Matthias Ettrich and the LyX Team
*/

#ifndef LYX_FILETOOL_H
#define LYX_FILETOOL_H

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdlib>
#include <fcntl.h>
#include <cerrno>

#include "debug.h"
#include "LString.h"
#include "support/lstrings.h"


///
string CreateBufferTmpDir (string const & pathfor = string());

/// Creates directory. Returns true on succes.
bool createDirectory(string const & name, int permissions);

///
string CreateLyXTmpDir (string const & deflt);

///
int DestroyBufferTmpDir (string const & tmpdir);

///
int DestroyLyXTmpDir (string const & tmpdir);

/** Find file by searching several directories.
  Uses a string of paths separated by ";"s to find a file to open.
    Can't cope with pathnames with a ';' in them. Returns full path to file.
    If path entry begins with $$LyX/, use system_lyxdir.
    If path entry begins with $$User/, use user_lyxdir.
    Example: "$$User/doc;$$LyX/doc".
*/
string FileOpenSearch (string const & path, string const & name, 
		       string const & ext = string());

/** Returns the real name of file name in directory path, with optional
  extension ext.
  The file is searched in the given path (unless it is an absolute
  file name), first directly, and then with extension .ext (if given).
  */
string FileSearch(string const & path, string const & name, 
		  string const & ext = string());

/** Is directory read only?
  returns 
    1: dir writeable
    0: not writeable
   -1: error- couldn't find out, or unsure
  */
int IsDirWriteable (string const & path);

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

/** Returns the path of a library data file.
  Search the file name.ext in the subdirectory dir of
  \begin{enumerate}
    \item user_lyxdir
    \item build_lyxdir (if not empty)
    \item system_lyxdir
  \end{enumerate}
    The third parameter `ext' is optional.
*/
string LibFileSearch(string const & dir, string const & name, 
		     string const & ext = string());

/** Same as LibFileSearch(), but tries first to find an
  internationalized version of the file by prepending $LANG_ to the
  name 
  */
string i18nLibFileSearch(string const & dir, string const & name, 
			 string const & ext = string());

///
string GetEnv(string const & envname);

/// A helper function.
string GetEnvPath(string const & name);

///
bool PutEnv(string const & envstr);

///
bool PutEnvPath(string const & envstr);

/// Substitutes active latex characters with underscores in filename
string MakeLatexName(string const & file);

/// Put the name in quotes suitable for the current shell
string QuoteName(string const & file);

/** Returns an unique name to be used as a temporary file. If given,
  'mask' should the prefix to the temporary file, the rest of the
  temporary filename will be made from the pid and three letters.
  */
string TmpFileName(string const & dir = string(), 
		   string const & mask = "lyx_tmp");

/// Is a filename/path absolute?
bool AbsolutePath(string const & path);

/// Add a filename to a path. Any path from filename is stripped first.
string AddName(string const & path, string const & fname);

/// Append sub-directory(ies) to path in an intelligent way
string AddPath(string const & path, string const & path2);

/** Change extension of oldname to extension.
 If no_path is true, the path is stripped from the filename.
 If oldname does not have an extension, it is appended.
 If the extension is empty, any extension is removed from the name.
 */
string ChangeExtension(string const & oldname, string const & extension, 
		       bool no_path);

/// Create absolute path. If impossible, don't do anything
string ExpandPath(string const & path);

/// gets current working directory
string GetCWD();


/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
string MakeAbsPath(string const & RelPath = string(), 
		   string const & BasePath = string());

/** Creates a nice compact path for displaying. The parameter
  threshold, if given, specifies the maximal length of the path.
  */
string MakeDisplayPath(string const & path, unsigned int threshold= 1000);

/** Makes relative path out of absolute path.
  If it is deeper than basepath,
  it's easy. If basepath and abspath share something (they are all deeper
  than some directory), it'll be rendered using ..'s. If they are completely
  different, then the absolute path will be used as relative path
  WARNING: the absolute path and base path must really be absolute paths!!!
  */
string MakeRelPath(string const & abspath, string const & basepath);

/// Strip filename from path name
string OnlyPath(string const & fname);

/// Normalize a path. Constracts path/../path
string NormalizePath(string const & path);

/// Strips path from filename
string OnlyFilename(string const & fname);

/// Cleanup a path if necessary. Currently only useful with OS/2
string CleanupPath(string const & path) ;

/** Check and Replace Environmentvariables ${NAME} in Path.
    Replaces all occurences of these, if they are found in the
    environment.
    Variables are defined by Var := '${' [a-zA-Z_][a-zA-Z_0-9]* '}'
*/
string ReplaceEnvironmentPath(string const & path);

/* Set Link to the path File Points to as a symbolic link.
   Return True if succesfull, False other wise */
bool LyXReadLink(string const & file, string & Link);

/* Uses kpsewhich to find tex files */
string findtexfile(string const & fil, string const & format);

#endif
