// -*- C++-*-
/* lyx-filetool.h : tools functions for file/path handling
   this file is part of LyX, the High Level Word Processor
   Copyright 1995-1999, Matthias Ettrich and the LyX Team
*/

#ifndef LYX_FILETOOL_H
#define LYX_FILETOOL_H

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <cerrno>
#include "debug.h"
#include "LString.h"
#include "support/lstrings.h"

/** A file class.
  Use this instead of FILE *, it gives a much better structure.
  It should prehaps do a bit more error checking than it does now.
  Currently it is a verbatim copy from p309 of Bjarne Stroupstrups
  The C++ Programming Language. + some additions.
 */
class FilePtr {
public:
	///
	enum file_mode {
		read,
		write,
		update,
		truncate
	};
	///
	FilePtr(string const & name, file_mode mode) {
		init();
		do_open(name, mode);
	}
	///
	FilePtr(FILE * pp) { init(); p = pp; }
	///
	~FilePtr() { close(); }

	/** Use this if you want to rebind the FilePtr to another file.
	 */
	FilePtr & reopen(string const & name, file_mode mode) {
		// close the file it it is already open
		close();
		// Now open the file.
		do_open(name, mode);

		return *this;
	}
	/** Close the file.
	    Use this with some carefullness. After it has been used
	    the FilePtr is unusable. Only use it if it is important
	    that the file is closed before the FilePtr goes out
	    of scope. */
	int close() { 
		if (p) {
			int result = fclose(p); 
			p = 0; 
			return result;
		} else 
			return 0;
	}
	/// automatic converson to FILE* if that is needed.
	operator FILE*() { return p; }
	///
	FilePtr & operator=(FILE * f) { p= f; return *this;}
	///
	FILE * operator()() { return p; }
private:
	///
	void do_open(string const & name, file_mode mode) {
		string modestr;
		
		switch(mode) {
			// do appropiate #ifdef here so support EMX
#ifndef __EMX__
		case read: modestr = "r"; break;
		case write: modestr = "w"; break;
#else
		case read: modestr = "rt"; break; // Can read both DOS & UNIX text files.
		case write: modestr = "w"; break; // Write UNIX text files.
#endif
			
		case update: modestr = "r+"; break;
		case truncate: modestr = "w+"; break;
		}
		// Should probably be rewritten to use open(2)
		if((p = fopen(name.c_str(), modestr.c_str()))) {
			// file succesfully opened.
			if (fcntl(fileno(p), F_SETFD, FD_CLOEXEC) == -1) {
				p = 0;
			}
		} else {
			// we have an error let's check what it is.
			switch(errno) {
			case EINVAL:
				// Internal LyX error.
				lyxerr << "FilePtr: Wrong parameter given to fopen." << endl;
				break;
			default:
				// unknown error
				break;
			}
		}
	}
	///
	void init() { p = 0; }
	///
	FILE * p;
};


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

///
bool IsSGMLFilename(string const & filename);

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

/// Substitutes spaces with underscores in filename (and path)
string SpaceLess(string const & file);

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
