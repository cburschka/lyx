/*
	filetools.C (former paths.C) - part of LyX project
	General path-mangling functions 
	Copyright (C) 1996 Ivan Schreter
	Parts Copyright (C) 1996 Dirk Niggemann
        Parts Copyright (C) 1985, 1990, 1993 Free Software Foundation, Inc.
	Parts Copyright (C) 1996 Asger Alstrup
	
	See also filetools.H.

	lyx-filetool.C : tools functions for file/path handling
	this file is part of LyX, the High Level Word Processor
	copyright (C) 1995-1996, Matthias Ettrich and the LyX Team

*/

#include <config.h>

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>        // I know it's OS/2 specific (SMiyata)

#ifdef __GNUG__
#pragma implementation "filetools.h"
#endif

#include "filetools.h"
#include "lyx_gui_misc.h"
#include "FileInfo.h"
#include "pathstack.h"        // I know it's OS/2 specific (SMiyata)
#include "gettext.h"

// Which part of this is still necessary? (JMarc).
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

// 	$Id: filetools.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: filetools.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */


extern LString system_lyxdir;
extern LString build_lyxdir;
extern LString user_lyxdir;
extern LString system_tempdir;


bool IsLyXFilename(LString const & filename)
{
	return filename.contains(".lyx");
}


bool IsSGMLFilename(LString const & filename)
{
	return filename.contains(".sgml");
}


// Substitutes spaces with underscores in filename (and path)
LString SpaceLess(LString const & file)
{
	LString name = OnlyFilename(file);
	LString path = OnlyPath(file);
	
	for (int i=0; i<name.length(); i++) {
		name[i] &= 0x7f;
		if (!isalnum(name[i]) && name[i] != '.')
			name[i] = '_';
	}
	LString temp = AddName(path, name);
	// Replace spaces with underscores, also in directory
	// No!!! I checked it that it is not necessary.
	// temp.subst(' ','_');

	return temp;
}


/// Returns an unique name to be used as a temporary file. 
LString TmpFileName(LString const & dir, LString const & mask)
{// With all these temporary variables, it should be safe enough :-) (JMarc)
	LString tmpdir;	
	if (dir.empty())
		tmpdir = system_tempdir;
	else
		tmpdir = dir;
	LString tmpfl = AddName(tmpdir, mask);

	// find a uniq postfix for the filename...
	// using the pid, and...
	tmpfl += int(getpid());
	// a short string...
	LString ret;
	FileInfo fnfo;
	for (int a='a'; a<= 'z'; a++)
		for (int b='a'; b<= 'z'; b++)
			for (int c='a'; c<= 'z'; c++) {
				// if this is not enough I have no idea what
				// to do.
				ret = tmpfl + char(a) + char(b) + char(c);
				// check if the file exist
				if (!fnfo.newFile(ret).exist())
					return ret;
			}
	lyxerr.print("Not able to find a uniq tmpfile name.");
	return LString();
}


// Is a file readable ?
bool IsFileReadable (LString const & path)
{
	FileInfo file(path);
	if (file.isOK() && file.isRegular() && file.readable())
		return true;
	else
		return false;
}


// Is a file read_only?
// return 1 read-write
//	  0 read_only
//	 -1 error (doesn't exist, no access, anything else) 
int IsFileWriteable (LString const & path)
{
	FilePtr fp(path, FilePtr::update);
	if (!fp()) {
		if ((errno == EACCES) || (errno == EROFS)) {
			//fp = FilePtr(path, FilePtr::read);
			fp.reopen(path, FilePtr::read);
			if (fp()) {
				return 0;
			}
		}
		return -1;
	}
	return 1;
}


//returns 1: dir writeable
//	  0: not writeable
//	 -1: error- couldn't find out
int IsDirWriteable (LString const & path)
{
        LString tmpfl = TmpFileName(path);

	if (tmpfl.empty()) {
		WriteFSAlert(_("LyX Internal Error!"), 
			     _("Could not test if directory is writeable"));
		return -1;
	} else {
	FilePtr fp(tmpfl, FilePtr::truncate);
	if (!fp()) {
		if (errno == EACCES) {
			return 0;
		} else { 
        		WriteFSAlert(_("LyX Internal Error!"), 
				     _("Cannot open directory test file"));
			return -1;
		}
		}
	}
		if (remove (tmpfl.c_str())) {
			WriteFSAlert(_("LyX Internal Error!"), 
				    _("Created test file but cannot remove it?"));
 			return -1;
	}
	return 1;
}


// Uses a string of paths separated by ";"s to find a file to open.
// Can't cope with pathnames with a ';' in them. Returns full path to file.
// If path entry begins with $$LyX/, use system_lyxdir
// If path entry begins with $$User/, use user_lyxdir
// Example: "$$User/doc;$$LyX/doc"
LString FileOpenSearch (LString const & path, LString const & name, 
			LString const & ext)
{
	LString real_file, path_element;
	LString tmppath = path;
	bool notfound = true;

	tmppath.split(path_element, ';');
	
	while (notfound && !path_element.empty()) {
		path_element = CleanupPath(path_element);
		if (!path_element.suffixIs('/'))
			path_element+='/';
		path_element.subst("$$LyX",system_lyxdir);
		path_element.subst("$$User",user_lyxdir);
		
		real_file = FileSearch(path_element, name, ext);

		if (real_file.empty()) {
		  tmppath.split(path_element, ';');
		} else {
		  notfound = false;
		}
	}
#ifdef __EMX__
	if (ext.empty() && notfound) {
		real_file = FileOpenSearch(path, name, "exe");
		if (notfound) real_file = FileOpenSearch(path, name, "cmd");
	}
#endif
	return real_file;
}


// Returns the real name of file name in directory path, with optional
// extension ext.  
LString FileSearch(LString const & path, LString const & name, 
		   LString const & ext)
{
        LString fullname;
	LString tmp;

	// if `name' is an absolute path, we ignore the setting of `path'
	// Expand Environmentvariables in 'name'
	LString tmpname = ReplaceEnvironmentPath(name);
	fullname = MakeAbsPath(tmpname,path);

	// search first without extension, then with it.
	if (IsFileReadable(fullname))
		return fullname;
	else if(ext.empty()) 
		return LString();
	else { // Is it not more reasonable to use ChangeExtension()? (SMiyata)
		fullname += '.';
		fullname += ext;
		if (IsFileReadable(fullname))
			return fullname;
		else 
			return LString();
	}
}


// Search the file name.ext in the subdirectory dir of
//   1) user_lyxdir
//   2) build_lyxdir (if not empty)
//   3) system_lyxdir
LString LibFileSearch(LString const & dir, LString const & name, 
		      LString const & ext)
{
        LString fullname = FileSearch(AddPath(user_lyxdir,dir), name,
				      ext); 
	if (!fullname.empty())
		return fullname;

	if (!build_lyxdir.empty()) 
	  fullname = FileSearch(AddPath(build_lyxdir,dir), 
				name, ext);
	if (!fullname.empty())
		return fullname;

	return FileSearch(AddPath(system_lyxdir,dir), name, ext);
}

LString i18nLibFileSearch(LString const & dir, LString const & name, 
			  LString const & ext)
{
	LString lang = LString(getenv("LANG")).token('_',0);

	if (lang.empty())
		return LibFileSearch(dir, name, ext);
	else {
		LString tmp = LibFileSearch(dir, lang + '_' + name,
					    ext);
		if (!tmp.empty())
			return tmp;
		else
			return LibFileSearch(dir, name, ext);
	}
}


static
int DeleteAllFilesInDir (LString const & path)
{
	DIR * dir;
	struct dirent *de;
	dir = opendir(path.c_str());
	if (!dir) {
		WriteFSAlert (_("Error! Cannot open directory:"), path);
		return -1;
	}
	while ((de = readdir(dir))) {
		LString temp = de->d_name;
		if (temp=="." || temp=="..") 
			continue;
		LString unlinkpath = AddName (path, temp);

		lyxerr.debug("Deleting file: " + unlinkpath);

 		if (remove (unlinkpath.c_str()))
			WriteFSAlert (_("Error! Could not remove file:"), 
				      unlinkpath);
        }
	closedir (dir);
	return 0;
}


static
LString CreateTmpDir (LString const & tempdir, LString const & mask)
{
	LString tmpfl = TmpFileName(tempdir, mask);
	
	if ((tmpfl.empty()) || mkdir (tmpfl.c_str(), 0777)) {
		WriteFSAlert(_("Error! Couldn't create temporary directory:"),
			     tempdir);
		return LString();
	}
	return MakeAbsPath(tmpfl);
}


static
int DestroyTmpDir (LString const & tmpdir, bool Allfiles)
{
	if ((Allfiles) && (DeleteAllFilesInDir (tmpdir))) return -1;
	if (rmdir(tmpdir.c_str())) { 
#ifdef __EMX__
		if (errno == EBUSY) {
			chdir(user_lyxdir.c_str()); // They are in the same drive.
			if (!rmdir(tmpdir.c_str())) return 0;
		}
#endif
		WriteFSAlert(_("Error! Couldn't delete temporary directory:"), 
			     tmpdir);
		return -1;
	}
	return 0; 
} 


LString CreateBufferTmpDir (LString const & pathfor)
{
	return CreateTmpDir (pathfor, "lyx_bufrtmp");
}


int DestroyBufferTmpDir (LString const & tmpdir)
{
       return DestroyTmpDir (tmpdir, true);
}


LString CreateLyXTmpDir (LString const & deflt)
{
        LString t;
        
	if ((!deflt.empty()) && (deflt!="/tmp")) {
		if (mkdir (deflt.c_str(), 0777)) {
#ifdef __EMX__
                        PathPush(user_lyxdir);
#endif
			t = CreateTmpDir (deflt.c_str(), "lyx_tmp");
#ifdef __EMX__
                        PathPop();
#endif
                        return t;
		} else
                        return deflt;
	} else {
#ifdef __EMX__
		PathPush(user_lyxdir);
#endif
		t = CreateTmpDir ("/tmp", "lyx_tmp");
#ifdef __EMX__
		PathPop();
#endif
		return t;
	}
}


int DestroyLyXTmpDir (LString const & tmpdir)
{
       return DestroyTmpDir (tmpdir, false); // Why false?
}


// Creates directory. Returns true if succesfull
bool createDirectory(LString const & path, int permission)
{
	LString temp = CleanupPath(path);
	
	// Cut off trailing /s
	temp.strip('/');

	if (temp.empty()) {
		WriteAlert(_("Internal error!"),
			   _("Call to createDirectory with invalid name"));
		return false;
	}

	if (mkdir(temp.c_str(), permission)) {
		WriteFSAlert (_("Error! Couldn't create directory:"), temp);
		return false;
	}
	return true;
}


// Returns current working directory
LString GetCWD ()
{
  	int n = 256;	// Assume path is less than 256 chars
	char * err;
  	char * tbuf = new char [n];
	LString result;
  	
  	// Safe. Hopefully all getcwds behave this way!
  	while (((err = getcwd (tbuf, n)) == NULL) && (errno == ERANGE)) {
		// Buffer too small, double the buffersize and try again
    		delete[] tbuf;
    		n = 2*n;
    		tbuf = new char [n];
  	}

	if (err) result = tbuf;
	delete[] tbuf;
	return result;
}


// Strip filename from path name
LString OnlyPath(LString const &Filename)
{
	// If empty filename, return empty
	if (Filename.empty()) return Filename;

	// Find last / or start of filename
	int j = Filename.length() - 1;
	LString temp = CleanupPath(Filename);
	for (; j > 0 && temp[j] != '/'; j--);

	if (temp[j] != '/')
		return "./";
	else {
		// Strip to pathname
		return temp.substring(0, j);
	}
}


// Convert relative path into absolute path based on a basepath.
// If relpath is absolute, just use that.
// If basepath is empty, use CWD as base.
LString MakeAbsPath(LString const &RelPath, LString const &BasePath)
{
	// checks for already absolute path
	if (AbsolutePath(RelPath))
#ifdef __EMX__
		if(RelPath[0]!='/' || RelPath[0]!='\\')
#endif
		return RelPath;

	// Copies given paths
	LString TempRel = CleanupPath(RelPath);

	LString TempBase;

	if (!BasePath.empty()) {
#ifndef __EMX__
		TempBase = BasePath;
#else
		char* with_drive = new char[_MAX_PATH];
		_abspath(with_drive, BasePath.c_str(), _MAX_PATH);
		TempBase = with_drive;
		delete[] with_drive;
#endif
	} else
		TempBase = GetCWD(); //safer_getcwd();
#ifdef __EMX__
	if (AbsolutePath(TempRel))
		return TempBase.substring(0,1) + TempRel;
#endif

	// Handle /./ at the end of the path
	while(TempBase.suffixIs("/./"))
		TempBase.substring(0,TempBase.length()-3);

	// processes relative path
	LString RTemp = TempRel;
	LString Temp;

	while (!RTemp.empty()) {
		// Split by next /
		RTemp.split(Temp, '/');
		
		if (Temp==".") continue;
		if (Temp=="..") {
			// Remove one level of TempBase
			int i = TempBase.length()-2;
#ifndef __EMX__
			if (i<0) i=0;
			while (i>0 && TempBase[i] != '/') i--;
			if (i>0)
#else
				if (i<2) i=2;
			while (i>2 && TempBase[i] != '/') i--;
			if (i>2)
#endif
				TempBase.substring(0, i);
			else
				TempBase += '/';
		} else {
			// Add this piece to TempBase
			if (!TempBase.suffixIs('/'))
				TempBase += '/';
			TempBase += Temp;
		}
	}

	// returns absolute path
	return TempBase;	
}


// Correctly append filename to the pathname.
// If pathname is '.', then don't use pathname.
// Chops any path of filename.
LString AddName(LString const &Path, LString const &FileName)
{
	// Get basename
	LString Basename = OnlyFilename(FileName);

	LString buf;

	if (Path != "." && Path != "./" && !Path.empty()) {
		buf = CleanupPath(Path);
		if (!Path.suffixIs('/'))
			buf += '/';
	}

	return buf + Basename;
}


// Strips path from filename
LString OnlyFilename(LString const &Filename)
{
	// If empty filename, return empty
	if (Filename.empty()) return Filename;

	int j;
	// Find last / or start of filename
	LString temp = CleanupPath(Filename);
	for (j=Filename.length()-1; temp[j] != '/' && j>0; j--);

	// Skip potential /
	if (j!=0) j++;

	// Strip to basename
	return temp.substring(j, temp.length()-1);
}


// Is a filename/path absolute?
bool AbsolutePath(LString const &path)
{
#ifndef __EMX__
	return (!path.empty() && path[0]=='/');
#else
	return (!path.empty() && (path[0]=='/' || (isalpha((unsigned char) path[0]) && path[1]==':')));
#endif
}


// Create absolute path. If impossible, don't do anything
// Supports ./ and ~/. Later we can add support for ~logname/. (Asger)
LString ExpandPath(LString const &path)
{
	// checks for already absolute path
	LString RTemp = ReplaceEnvironmentPath(path);
	if (AbsolutePath(RTemp))
		return RTemp;

	LString Temp;
	LString copy(RTemp);

	// Split by next /
	RTemp.split(Temp, '/');

	if (Temp==".") {
		return GetCWD() + '/' + RTemp;
	} else if (Temp=="~") {
		return getEnvPath("HOME") + '/' + RTemp;
	} else if (Temp=="..") {
		return MakeAbsPath(copy);
	} else
		// Don't know how to handle this
		return copy;
}


// Normalize a path
// Constracts path/../path
// Can't handle "../../" or "/../" (Asger)
LString NormalizePath(LString const &path)
{
	LString TempBase;
	LString RTemp;
	LString Temp;

	if (AbsolutePath(path))
		RTemp = path;
	else
		// Make implicit current directory explicit
		RTemp = "./" +path;

	while (!RTemp.empty()) {
		// Split by next /
		RTemp.split(Temp, '/');
		
		if (Temp==".") {
			TempBase = "./";
		} else if (Temp=="..") {
			// Remove one level of TempBase
			int i = TempBase.length()-2;
			while (i>0 && TempBase[i] != '/')
				i--;
			if (i>=0 && TempBase[i] == '/')
				TempBase.substring(0, i);
			else
				TempBase = "../";
		} else {
			TempBase += Temp + '/';
		}
	}

	// returns absolute path
	return TempBase;	
}

LString CleanupPath(LString const &path) 
{
#ifdef __EMX__	  /* SMiyata: This should fix searchpath bug. */
	LString temppath(path);
	temppath.subst('\\', '/');
	temppath.subst("//", "/");
	temppath.lowercase();
	return temppath;
#else // On unix, nothing to do
	return path;
#endif
}


//
// Search ${...} as Variable-Name inside the string and replace it with
// the denoted environmentvariable
// Allow Variables according to 
//  variable :=  '$' '{' [A-Za-z_]{[A-Za-z_0-9]*} '}'
//

LString ReplaceEnvironmentPath(LString const &path)
{
// 
// CompareChar: Environmentvariables starts with this character
// PathChar:    Next path component start with this character
// while CompareChar found do:
//       Split String with PathChar
// 	 Search Environmentvariable
//	 if found: Replace Strings
//
	const char CompareChar = '$';
	const char FirstChar = '{'; 
	const char EndChar = '}'; 
	const char UnderscoreChar = '_'; 
	const LString EndString(EndChar);
	const LString FirstString(FirstChar);
	const LString CompareString(CompareChar);
	const LString RegExp("*}*"); // Exist EndChar inside a String?

	if (path.empty()) return path; // nothing to do.

// first: Search for a '$' - Sign.
	LString copy(path);
    LString result1(copy);    // for split-calls
	LString result0 = copy.split(result1, CompareChar);
	while (!result0.empty()) {
		LString copy1(result0); // contains String after $
		
		// Check, if there is an EndChar inside original String.
		
		if (!copy1.regexMatch(RegExp)) {
			// No EndChar inside. So we are finished
			result1 += CompareString + result0;
			result0 = LString();
			continue;
		}

		LString res1;
		LString res0 = copy1.split(res1, EndChar);
		// Now res1 holds the environmentvariable
		// First, check, if Contents is ok.
		if (res1.empty()) { // No environmentvariable. Continue Loop.
			result1 += CompareString + FirstString;
			result0  = res0;
			continue;
		}
		// check contents of res1
		const char *res1_contents = res1.c_str();
		if (*res1_contents != FirstChar) {
			// Again No Environmentvariable
			result1 += CompareString;
			result0  = res0;
		}

		// Check for variable names
		// Situation ${} is detected as "No Environmentvariable"
		const char *cp1 = res1_contents+1;
		bool result = isalpha((unsigned char) *cp1) || (*cp1 == UnderscoreChar);
		++cp1;
		while (*cp1 && result) {
			result = isalnum((unsigned char) *cp1) || 
				(*cp1 == UnderscoreChar); 
			++cp1;
		}

		if (!result) {
			// no correct variable name
			result1 += CompareString + res1 + EndString;
			result0  = res0.split(res1, CompareChar);
			result1 += res1;
			continue;
		}
            
		char *env = getenv(res1_contents+1);
		if (env) {
			// Congratulations. Environmentvariable found
			result1 += env;
		} else {
			result1 += CompareString + res1 + EndString;
		}
		// Next $-Sign?
		result0  = res0.split(res1, CompareChar);
		result1 += res1;
	} 
	return result1;
}  // ReplaceEnvironmentPath


// Make relative path out of two absolute paths
LString MakeRelPath(LString const & abspath0, LString const & basepath0)
// Makes relative path out of absolute path. If it is deeper than basepath,
// it's easy. If basepath and abspath share something (they are all deeper
// than some directory), it'll be rendered using ..'s. If they are completely
// different, then the absolute path will be used as relative path.
{
	// This is a hack. It should probaly be done in another way. Lgb.
	LString abspath = CleanupPath(abspath0);
	LString basepath = CleanupPath(basepath0);
	if (abspath.empty())
		return "<unknown_path>";

	const int abslen = abspath.length();
	const int baselen = basepath.length();
	
	// Find first different character
	int i = 0;
	while (i < abslen && i < baselen && abspath[i] == basepath[i]) ++i;

	// Go back to last /
	if (i < abslen && i < baselen
	    || (i<abslen && abspath[i] != '/' && i==baselen)
	    || (i<baselen && basepath[i] != '/' && i==abslen))
	{
		if (i) --i;	// here was the last match
		while (i && abspath[i] != '/') --i;
	}

	if (i == 0) {
		// actually no match - cannot make it relative
		return abspath;
	}

	// Count how many dirs there are in basepath above match
	// and append as many '..''s into relpath
	LString buf;
	int j = i;
	while (j < baselen) {
		if (basepath[j] == '/') {
			if (j+1 == baselen) break;
			buf += "../";
		}
		++j;
	}

	// Append relative stuff from common directory to abspath
	if (abspath[i] == '/') ++i;
	for (; i<abslen; ++i)
		buf += abspath[i];
	// Remove trailing /
	if (buf.suffixIs('/'))
		buf.substring(0,buf.length()-2);
	// Substitute empty with .
	if (buf.empty())
		buf = '.';
	return buf;
}


// Append sub-directory(ies) to a path in an intelligent way
LString AddPath(LString const & path, LString const & path_2)
{
	LString buf;
	LString path2 = CleanupPath(path_2);

	if (!path.empty() && path != "." && path != "./") {
		buf = CleanupPath(path);
		if (!path.suffixIs('/'))
			buf += '/';
	}

	if (!path2.empty()){
	        int p2start = 0;
		while (path2[p2start] == '/') p2start++;

		int p2end = path2.length()-1;
		while (path2[p2end] == '/') p2end--;

		LString tmp = path2;
		tmp.substring(p2start,p2end);
		buf += tmp + '/';
	}
	return buf;
}


/* 
 Change extension of oldname to extension.
 Strips path off if no_path == true.
 If no extension on oldname, just appends.
 */
LString ChangeExtension(LString const & origname, LString const & extension, 
			bool no_path) 
{ 
	LString oldname = CleanupPath(origname);
	int n = oldname.length()-1;
	int dot;

	// Make sure the extension includes the dot, if not empty
	LString ext;
	if (!extension.empty() && extension[0] != '.')
		ext = "." + extension;
	else
		ext = extension;

	// Go back to the first dot not crossing any /
	for (dot=n; dot>=0 && oldname[dot]!='.' && oldname[dot]!='/'; dot--);
   
	if (dot==-1 || oldname[dot]!='.')
		// If no extension was found, we use the end of the string
		dot = n;
	else
		// Remove the dot, because the new extension includes it
		dot--;

	// "path" points to last / or 0 if path is wanted
	int path = 0;
	if (no_path) {
		for (path=dot; path && oldname[path]!='/';path--);
		if (oldname[path]=='/')
			path++;
	} else 
		path = 0;

	LString result = oldname;
	result.substring(path,dot);
	if (!ext.empty())
		result += ext;
	return result;
}


// Creates a nice compact path for displaying
LString MakeDisplayPath (LString const & path, int threshold)
{
	const int l1 = path.length();

	// First, we try a relative path compared to home
	LString home = getEnvPath("HOME");
	LString relhome = MakeRelPath(path, home);

	int l2 = relhome.length();

	LString prefix;

	// If we backup from home or don't have a relative path,
	// this try is no good
	if (relhome.prefixIs("../") || AbsolutePath(relhome)) {
		// relative path was no good, just use the original path
		relhome = path;
		l2 = l1;
	} else {
		prefix = "~/";
	}

	// Is the path too long?
	if (l2 > threshold) {
		// Yes, shortend it
		prefix += ".../";
		
		LString temp;
		
		while (relhome.length()>threshold)
			relhome.split(temp, '/');

		// Did we shortend everything away?
		if (relhome.empty()) {
			// Yes, filename in itself is too long.
			// Pick the start and the end of the filename.
			relhome = OnlyFilename(path);
			LString head = relhome;
			head.substring(0, threshold/2 - 3);

			LString tail = relhome;
			l2 = tail.length();
			tail.substring(l2 - threshold/2 -2, l2 - 1);

			relhome = head + "..." + tail;
		}
	}
	return prefix + relhome;
}

bool LyXReadLink(LString const & File, LString & Link)
{
	char LinkBuffer[512];
                // Should be PATH_MAX but that needs autconf support
	int nRead;
	nRead = readlink(File.c_str(), LinkBuffer,sizeof(LinkBuffer)-1);
	if (nRead <= 0)
		return false;
	LinkBuffer[nRead] = 0;
	Link = LinkBuffer;
	return true;
}
