// -*- C++-*-
/* lyx-filetool.h : tools functions for file/path handling
   this file is part of LyX, the High Level Word Processor
   copyright (C) 1995-1997, Matthias Ettrich and the LyX Team
*/

#ifndef __LYX_FILETOOL_H__
#define __LYX_FILETOOL_H__

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "error.h"
#include "LString.h"

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
	FilePtr(LString const &name, file_mode mode)
	{
		init();
		do_open(name, mode);
	}
	///
	FilePtr(FILE *pp) { init(); p = pp; }
	///
	~FilePtr() { close(); }

	/** Use this if you want to rebind the FilePtr to another file.
	 */
	FilePtr& reopen(LString const &name, file_mode mode) {
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
	FilePtr& operator=(FILE *f) { p=f; return *this;}
	///
	FILE *operator()() { return p; }
private:
	///
	void do_open(LString const &name, file_mode mode) {
		char modestr[3];
		
		switch(mode) {
			// do appropiate #ifdef here so support EMX
#ifndef __EMX__
		case read: strcpy(modestr, "r"); break;
		case write: strcpy(modestr, "w"); break;
#else
		case read: strcpy(modestr,"rt"); break; // Can read both DOS & UNIX text files.
		case write: strcpy(modestr,"w"); break; // Write UNIX text files.
#endif
			
		case update: strcpy(modestr, "r+"); break;
		case truncate: strcpy(modestr, "w+"); break;
		}
		// Should probably be rewritten to use open(2)
		if((p = fopen(name.c_str(), modestr))) {
			// file succesfully opened.
			if (fcntl(fileno(p),F_SETFD,FD_CLOEXEC) == -1) {
				p = 0;
			}
		} else {
			// we have an error let's check what it is.
			switch(errno) {
			case EINVAL:
				// Internal LyX error.
				lyxerr.print("FilePtr: Wrong parameter given to fopen.");
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
	FILE *p;
};


///
LString CreateBufferTmpDir (LString const & pathfor = LString());

/// Creates directory. Returns true on succes.
bool createDirectory(LString const & name, int permissions);

///
LString CreateLyXTmpDir (LString const & deflt);

///
int DestroyBufferTmpDir (LString const & tmpdir);

///
int DestroyLyXTmpDir (LString const & tmpdir);

/** Find file by searching several directories.
  Uses a string of paths separated by ";"s to find a file to open.
    Can't cope with pathnames with a ';' in them. Returns full path to file.
    If path entry begins with $$LyX/, use system_lyxdir.
    If path entry begins with $$User/, use user_lyxdir.
    Example: "$$User/doc;$$LyX/doc".
*/
LString FileOpenSearch (LString const & path, LString const & name, 
			LString const & ext = LString());

/** Returns the real name of file name in directory path, with optional
  extension ext.
  The file is searched in the given path (unless it is an absolute
  file name), first directly, and then with extension .ext (if given).
  */
LString FileSearch(LString const & path, LString const & name, 
		   LString const & ext = LString());

/** Is directory read only?
  returns 
    1: dir writeable
    0: not writeable
   -1: error- couldn't find out, or unsure
  */
int IsDirWriteable (LString const & path);

/** Is a file readable ?
  Returns true if the file `path' is readable.
 */
bool IsFileReadable (LString const & path);

/** Is file read only?
  returns
    1: read-write
    0: read_only
   -1: error (doesn't exist, no access, anything else)
  */
int IsFileWriteable (LString const & path);

///
bool IsLyXFilename(LString const & filename);

///
bool IsSGMLFilename(LString const & filename);

/** Returns the path of a library data file.
  Search the file name.ext in the subdirectory dir of
  \begin{enumerate}
    \item user_lyxdir
    \item build_lyxdir (if not empty)
    \item system_lyxdir
  \end{enumerate}
    The third parameter `ext' is optional.
*/
LString LibFileSearch(LString const & dir, LString const & name, 
		      LString const & ext = LString());

/** Same as LibFileSearch(), but tries first to find an
  internationalized version of the file by prepending $LANG_ to the
  name 
  */
LString i18nLibFileSearch(LString const & dir, LString const & name, 
			  LString const & ext = LString());

/// Substitutes spaces with underscores in filename (and path)
LString SpaceLess(LString const & file);

/** Returns an unique name to be used as a temporary file. If given,
  'mask' should the prefix to the temporary file, the rest of the
  temporary filename will be made from the pid and three letters.
  */
LString TmpFileName(LString const & dir = LString(), 
		    LString const & mask = "lyx_tmp");

/// Is a filename/path absolute?
bool AbsolutePath(LString const &path);

/// Add a filename to a path. Any path from filename is stripped first.
LString AddName(LString const &Path, LString const &Filename);

/// Append sub-directory(ies) to path in an intelligent way
LString AddPath(LString const & path, LString const & path2);

/** Change extension of oldname to extension.
 If no_path is true, the path is stripped from the filename.
 If oldname does not have an extension, it is appended.
 If the extension is empty, any extension is removed from the name.
 */
LString ChangeExtension(LString const & oldname, LString const & extension, 
			bool no_path);

/// Create absolute path. If impossible, don't do anything
LString ExpandPath(LString const &path);

/// gets current working directory
LString GetCWD();

/// A helper function.
inline LString getEnvPath(char const *name)
{
	LString pathlist;
	pathlist = getenv(name);
#ifndef __EMX__
	pathlist.subst(':', ';');
#else
	pathlist.subst('\\', '/');
#endif
	return pathlist.strip(';');
}

/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
LString MakeAbsPath(LString const &RelPath = LString(), 
		    LString const &BasePath = LString());

/** Creates a nice compact path for displaying. The parameter
  threshold, if given, specifies the maximal length of the path.
  */
LString MakeDisplayPath(LString const & path, int threshold=1000);

/** Makes relative path out of absolute path.
  If it is deeper than basepath,
  it's easy. If basepath and abspath share something (they are all deeper
  than some directory), it'll be rendered using ..'s. If they are completely
  different, then the absolute path will be used as relative path
  WARNING: the absolute path and base path must really be absolute paths!!!
  */
LString MakeRelPath(LString const & abspath, LString const & basepath);

/// Strip filename from path name
LString OnlyPath(LString const &Filename);

/// Normalize a path. Constracts path/../path
LString NormalizePath(LString const &path);

/// Strips path from filename
LString OnlyFilename(LString const &Filename);

/// Cleanup a path if necessary. Currently only useful with OS/2
LString CleanupPath(LString const &path) ;

/** Check and Replace Environmentvariables ${NAME} in Path.
    Replaces all occurences of these, if they are found in the
    environment.
    Variables are defined by Var := '${' [a-zA-Z_][a-zA-Z_0-9]* '}'
*/
LString ReplaceEnvironmentPath(LString const &path);

/* Set Link to the path File Points to as a symbolic link.
   Return True if succesfull, False other wise */
bool LyXReadLink(LString const & file, LString & Link);

#endif
