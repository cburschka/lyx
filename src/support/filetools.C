/*
	filetools.C (former paths.C) - part of LyX project
	General path-mangling functions 
	Copyright 1996 Ivan Schreter
	Parts Copyright 1996 Dirk Niggemann
        Parts Copyright 1985, 1990, 1993 Free Software Foundation, Inc.
	Parts Copyright 1996 Asger Alstrup
	
	See also filetools.H.

	lyx-filetool.C : tools functions for file/path handling
	this file is part of LyX, the High Level Word Processor
	Copyright 1995-1996, Matthias Ettrich and the LyX Team

*/

#include <config.h>

#include <cctype>

#include <utility>
using std::make_pair;
using std::pair;

#ifdef __GNUG__
#pragma implementation "filetools.h"
#endif

#include "filetools.h"
#include "LSubstring.h"
#include "lyx_gui_misc.h"
#include "FileInfo.h"
#include "support/path.h"        // I know it's OS/2 specific (SMiyata)
#include "gettext.h"
#include "lyxlib.h"

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

extern string system_lyxdir;
extern string build_lyxdir;
extern string user_lyxdir;
extern string system_tempdir;


bool IsLyXFilename(string const & filename)
{
	return contains(filename, ".lyx");
}


// Substitutes spaces with underscores in filename (and path)
string MakeLatexName(string const & file)
{
	string name = OnlyFilename(file);
	string path = OnlyPath(file);
	
	for (string::size_type i = 0; i < name.length(); ++i) {
		name[i] &= 0x7f; // set 8th bit to 0
	};

	// ok so we scan through the string twice, but who cares.
	string keep("abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"@!\"'()*+,-./0123456789:;<=>?[]`|");
	
	string::size_type pos = 0;
	while ((pos = name.find_first_not_of(keep, pos)) != string::npos) {
		name[pos++] = '_';
	}
	return AddName(path, name);
}

// Substitutes spaces with underscores in filename (and path)
string QuoteName(string const & name)
{
#ifdef WITH_WARNINGS
#warning Add proper emx support here!
#endif
#ifndef __EMX__
	return '\'' + name + '\'';
#else
	return name; 
#endif
}


/// Returns an unique name to be used as a temporary file. 
string TmpFileName(string const & dir, string const & mask)
{// With all these temporary variables, it should be safe enough :-) (JMarc)
	string tmpdir;	
	if (dir.empty())
		tmpdir = system_tempdir;
	else
		tmpdir = dir;
	string tmpfl = AddName(tmpdir, mask);

	// find a uniq postfix for the filename...
	// using the pid, and...
	tmpfl += tostr(getpid());
	// a short string...
	string ret;
	FileInfo fnfo;
	for (int a = 'a'; a <= 'z'; ++a)
		for (int b = 'a'; b <= 'z'; ++b)
			for (int c = 'a'; c <= 'z'; ++c) {
				// if this is not enough I have no idea what
				// to do.
				ret = tmpfl + char(a) + char(b) + char(c);
				// check if the file exist
				if (!fnfo.newFile(ret).exist())
					return ret;
			}
	lyxerr << "Not able to find a uniq tmpfile name." << endl;
	return string();
}


// Is a file readable ?
bool IsFileReadable (string const & path)
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
int IsFileWriteable (string const & path)
{
	FileInfo fi(path);
	if (fi.access(FileInfo::wperm|FileInfo::rperm)) // read-write
		return 1;
	if (fi.readable()) // read-only
		return 0;
	return -1; // everything else.
}


//returns 1: dir writeable
//	  0: not writeable
//	 -1: error- couldn't find out
int IsDirWriteable (string const & path)
{
        string tmpfl = TmpFileName(path);

	if (tmpfl.empty()) {
		WriteFSAlert(_("LyX Internal Error!"), 
			     _("Could not test if directory is writeable"));
		return -1;
	} else {
		FileInfo fi(path);
		if (fi.writable()) return 1;
		return 0;
	}
}


// Uses a string of paths separated by ";"s to find a file to open.
// Can't cope with pathnames with a ';' in them. Returns full path to file.
// If path entry begins with $$LyX/, use system_lyxdir
// If path entry begins with $$User/, use user_lyxdir
// Example: "$$User/doc;$$LyX/doc"
string FileOpenSearch (string const & path, string const & name, 
		       string const & ext)
{
	string real_file, path_element;
	bool notfound = true;
	string tmppath = split(path, path_element, ';');
	
	while (notfound && !path_element.empty()) {
		path_element = CleanupPath(path_element);
		if (!suffixIs(path_element, '/'))
			path_element+= '/';
		path_element = subst(path_element, "$$LyX", system_lyxdir);
		path_element = subst(path_element, "$$User", user_lyxdir);
		
		real_file = FileSearch(path_element, name, ext);
		
		if (real_file.empty()) {
			do {
				tmppath = split(tmppath, path_element, ';');
			} while(!tmppath.empty() && path_element.empty());
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
string FileSearch(string const & path, string const & name, 
		  string const & ext)
{
	// if `name' is an absolute path, we ignore the setting of `path'
	// Expand Environmentvariables in 'name'
	string tmpname = ReplaceEnvironmentPath(name);
	string fullname = MakeAbsPath(tmpname, path);
	
	// search first without extension, then with it.
	if (IsFileReadable(fullname))
		return fullname;
	else if (ext.empty()) 
		return string();
	else { // Is it not more reasonable to use ChangeExtension()? (SMiyata)
		fullname += '.';
		fullname += ext;
		if (IsFileReadable(fullname))
			return fullname;
		else 
			return string();
	}
}


// Search the file name.ext in the subdirectory dir of
//   1) user_lyxdir
//   2) build_lyxdir (if not empty)
//   3) system_lyxdir
string LibFileSearch(string const & dir, string const & name, 
		     string const & ext)
{
        string fullname = FileSearch(AddPath(user_lyxdir, dir),
				     name, ext); 
	if (!fullname.empty())
		return fullname;
	
	if (!build_lyxdir.empty()) 
		fullname = FileSearch(AddPath(build_lyxdir, dir), 
				      name, ext);
	if (!fullname.empty())
		return fullname;
	
	return FileSearch(AddPath(system_lyxdir, dir), name, ext);
}


string i18nLibFileSearch(string const & dir, string const & name, 
			 string const & ext)
{
	string lang = token(string(GetEnv("LANG")), '_', 0);
	
	if (lang.empty() || lang == "C")
		return LibFileSearch(dir, name, ext);
	else {
		string tmp = LibFileSearch(dir, lang + '_' + name,
					   ext);
		if (!tmp.empty())
			return tmp;
		else
			return LibFileSearch(dir, name, ext);
	}
}


string GetEnv(string const & envname)
{
        // f.ex. what about error checking?
        char const * const ch = getenv(envname.c_str());
        string envstr = !ch ? "" : ch;
        return envstr;
}


string GetEnvPath(string const & name)
{
#ifndef __EMX__
        string pathlist = subst(GetEnv(name), ':', ';');
#else
        string pathlist = subst(GetEnv(name), '\\', '/');
#endif
        return strip(pathlist, ';');
}


bool PutEnv(string const & envstr)
{
#ifdef WITH_WARNINGS
#warning Look at and fix this.
#endif
        // f.ex. what about error checking?
#if HAVE_PUTENV
        // this leaks, but what can we do about it?
        //   Is doing a getenv() and a free() of the older value 
        //   a good idea? (JMarc)
	// Actually we don't have to leak...calling putenv like this
	// should be enough: ... and this is obviously not enough if putenv
	// does not make a copy of the string. It is also not very wise to
	// put a string on the free store. If we have to leak we should do it
	// like this:
	char * leaker = new char[envstr.length() + 1];
	envstr.copy(leaker, envstr.length());
	leaker[envstr.length()] = '\0';
	int retval = lyx::putenv(leaker);

	// If putenv does not make a copy of the char const * this
	// is very dangerous. OTOH if it does take a copy this is the
	// best solution.
	// The  only implementation of putenv that I have seen does not
	// allocate memory. _And_ after testing the putenv in glibc it
	// seems that we need to make a copy of the string contents.
	// I will enable the above.
	//int retval = lyx::putenv(envstr.c_str());
#else
#ifdef HAVE_SETENV 
        string varname;
        string str = envstr.split(varname,'=');
        int retval = setenv(varname.c_str(), str.c_str(), true);
#endif
#endif
        return retval == 0;
}


bool PutEnvPath(string const & envstr)
{
        return PutEnv(envstr);
}


static
int DeleteAllFilesInDir (string const & path)
{
	// I have decided that we will be using parts from the boost
	// library. Check out http://www.boost.org/
	// For directory access we will then use the directory_iterator.
	// Then the code will be something like:
	// directory_iterator dit(path.c_str());
	// if (<some way to detect failure>) {
	//         WriteFSAlert(_("Error! Cannot open directory:"), path);
	//         return -1;
	// }
	// for (; dit != <someend>; ++dit) {
	//         if ((*dit) == 2." || (*dit) == "..")
	//                 continue;
	//         string unlinkpath = AddName(path, temp);
	//         if (remove(unlinkpath.c_str()))
	//                 WriteFSAlert(_("Error! Could not remove file:"),
	//                              unlinkpath);
	// }
	// return 0;
	DIR * dir = opendir(path.c_str());
	if (!dir) {
		WriteFSAlert (_("Error! Cannot open directory:"), path);
		return -1;
	}
	struct dirent * de;
	while ((de = readdir(dir))) {
		string temp = de->d_name;
		if (temp == "." || temp == "..") 
			continue;
		string unlinkpath = AddName (path, temp);

		lyxerr.debug() << "Deleting file: " << unlinkpath << endl;

 		if (remove(unlinkpath.c_str()))
			WriteFSAlert (_("Error! Could not remove file:"), 
				      unlinkpath);
        }
	closedir(dir);
	return 0;
}


static
string CreateTmpDir (string const & tempdir, string const & mask)
{
	string tmpfl = TmpFileName(tempdir, mask);
	
	if ((tmpfl.empty()) || lyx::mkdir (tmpfl.c_str(), 0777)) {
		WriteFSAlert(_("Error! Couldn't create temporary directory:"),
			     tempdir);
		return string();
	}
	return MakeAbsPath(tmpfl);
}


static
int DestroyTmpDir (string const & tmpdir, bool Allfiles)
{
#ifdef __EMX__
	Path p(user_lyxdir);
#endif
	if (Allfiles && DeleteAllFilesInDir(tmpdir)) return -1;
	if (rmdir(tmpdir.c_str())) { 
		WriteFSAlert(_("Error! Couldn't delete temporary directory:"), 
			     tmpdir);
		return -1;
	}
	return 0; 
} 


string CreateBufferTmpDir (string const & pathfor)
{
	return CreateTmpDir(pathfor, "lyx_bufrtmp");
}


int DestroyBufferTmpDir (string const & tmpdir)
{
	return DestroyTmpDir(tmpdir, true);
}


string CreateLyXTmpDir (string const & deflt)
{
	if ((!deflt.empty()) && (deflt  != "/tmp")) {
		if (lyx::mkdir(deflt.c_str(), 0777)) {
#ifdef __EMX__
                        Path p(user_lyxdir);
#endif
			string t = CreateTmpDir (deflt.c_str(), "lyx_tmp");
                        return t;
		} else
                        return deflt;
	} else {
#ifdef __EMX__
		Path p(user_lyxdir);
#endif
		string t = CreateTmpDir ("/tmp", "lyx_tmp");
		return t;
	}
}


int DestroyLyXTmpDir (string const & tmpdir)
{
       return DestroyTmpDir (tmpdir, false); // Why false?
}


// Creates directory. Returns true if succesfull
bool createDirectory(string const & path, int permission)
{
	string temp = strip(CleanupPath(path), '/');

	if (temp.empty()) {
		WriteAlert(_("Internal error!"),
			   _("Call to createDirectory with invalid name"));
		return false;
	}

	if (lyx::mkdir(temp.c_str(), permission)) {
		WriteFSAlert (_("Error! Couldn't create directory:"), temp);
		return false;
	}
	return true;
}


// Returns current working directory
string GetCWD ()
{
  	int n = 256;	// Assume path is less than 256 chars
	char * err;
  	char * tbuf = new char[n];
  	
  	// Safe. Hopefully all getcwds behave this way!
  	while (((err = lyx::getcwd (tbuf, n)) == 0) && (errno == ERANGE)) {
		// Buffer too small, double the buffersize and try again
    		delete[] tbuf;
    		n = 2 * n;
    		tbuf = new char[n];
  	}

	string result;
	if (err) result = tbuf;
	delete[] tbuf;
	return result;
}


// Strip filename from path name
string OnlyPath(string const & Filename)
{
	// If empty filename, return empty
	if (Filename.empty()) return Filename;

	// Find last / or start of filename
	string::size_type j = Filename.rfind('/');
	if (j == string::npos)
		return "./";
	return Filename.substr(0, j + 1);
}


// Convert relative path into absolute path based on a basepath.
// If relpath is absolute, just use that.
// If basepath is empty, use CWD as base.
string MakeAbsPath(string const & RelPath, string const & BasePath)
{
	// checks for already absolute path
	if (AbsolutePath(RelPath))
#ifdef __EMX__
		if(RelPath[0]!= '/' && RelPath[0]!= '\\')
#endif
		return RelPath;

	// Copies given paths
	string TempRel = CleanupPath(RelPath);

	string TempBase;

	if (!BasePath.empty()) {
#ifndef __EMX__
		TempBase = BasePath;
#else
		char * with_drive = new char[_MAX_PATH];
		_abspath(with_drive, BasePath.c_str(), _MAX_PATH);
		TempBase = with_drive;
		delete[] with_drive;
#endif
	} else
		TempBase = GetCWD();
#ifdef __EMX__
	if (AbsolutePath(TempRel))
		return TempBase.substr(0, 2) + TempRel;
#endif

	// Handle /./ at the end of the path
	while(suffixIs(TempBase, "/./"))
		TempBase.erase(TempBase.length() - 2);

	// processes relative path
	string RTemp = TempRel;
	string Temp;

	while (!RTemp.empty()) {
		// Split by next /
		RTemp = split(RTemp, Temp, '/');
		
		if (Temp == ".") continue;
		if (Temp == "..") {
			// Remove one level of TempBase
			int i = TempBase.length() - 2;
#ifndef __EMX__
			if (i < 0) i = 0;
			while (i > 0 && TempBase[i] != '/') --i;
			if (i > 0)
#else
			if (i < 2) i = 2;
			while (i > 2 && TempBase[i] != '/') --i;
			if (i > 2)
#endif
				TempBase.erase(i, string::npos);
			else
				TempBase += '/';
		} else {
			// Add this piece to TempBase
			if (!suffixIs(TempBase, '/'))
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
string AddName(string const & path, string const & fname)
{
	// Get basename
	string basename = OnlyFilename(fname);

	string buf;

	if (path != "." && path != "./" && !path.empty()) {
		buf = CleanupPath(path);
		if (!suffixIs(path, '/'))
			buf += '/';
	}

	return buf + basename;
}


// Strips path from filename
string OnlyFilename(string const & fname)
{
	if (fname.empty())
		return fname;

	string::size_type j = fname.rfind('/');
	if (j == string::npos) // no '/' in fname
		return fname;

	// Strip to basename
	return fname.substr(j + 1);
}


// Is a filename/path absolute?
bool AbsolutePath(string const & path)
{
#ifndef __EMX__
	return (!path.empty() && path[0] == '/');
#else
	return (!path.empty() && (path[0] == '/' || (isalpha(static_cast<unsigned char>(path[0])) && path.length()>1 && path[1] == ':')));
#endif
}


// Create absolute path. If impossible, don't do anything
// Supports ./ and ~/. Later we can add support for ~logname/. (Asger)
string ExpandPath(string const & path)
{
	// checks for already absolute path
	string RTemp = ReplaceEnvironmentPath(path);
	if (AbsolutePath(RTemp))
		return RTemp;

	string Temp;
	string copy(RTemp);

	// Split by next /
	RTemp= split(RTemp, Temp, '/');

	if (Temp == ".") {
		return GetCWD() + '/' + RTemp;
	} else if (Temp == "~") {
		return GetEnvPath("HOME") + '/' + RTemp;
	} else if (Temp == "..") {
		return MakeAbsPath(copy);
	} else
		// Don't know how to handle this
		return copy;
}


// Normalize a path
// Constracts path/../path
// Can't handle "../../" or "/../" (Asger)
string NormalizePath(string const & path)
{
	string TempBase;
	string RTemp;
	string Temp;

	if (AbsolutePath(path))
		RTemp = path;
	else
		// Make implicit current directory explicit
		RTemp = "./" +path;

	while (!RTemp.empty()) {
		// Split by next /
		RTemp = split(RTemp, Temp, '/');
		
		if (Temp == ".") {
			TempBase = "./";
		} else if (Temp == "..") {
			// Remove one level of TempBase
			int i = TempBase.length() - 2;
			while (i > 0 && TempBase[i] != '/')
				--i;
			if (i >= 0 && TempBase[i] == '/')
				TempBase.erase(i + 1, string::npos);
			else
				TempBase = "../";
		} else {
			TempBase += Temp + '/';
		}
	}

	// returns absolute path
	return TempBase;	
}

string CleanupPath(string const & path) 
{
#ifdef __EMX__	  /* SMiyata: This should fix searchpath bug. */
	string temppath = subst(path, '\\', '/');
	temppath = subst(temppath, "//", "/");
	return lowercase(temppath);
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

string ReplaceEnvironmentPath(string const & path)
{
// 
// CompareChar: Environmentvariables starts with this character
// PathChar:    Next path component start with this character
// while CompareChar found do:
//       Split String with PathChar
// 	 Search Environmentvariable
//	 if found: Replace Strings
//
	char const CompareChar = '$';
	char const FirstChar = '{'; 
	char const EndChar = '}'; 
	char const UnderscoreChar = '_'; 
	string EndString; EndString += EndChar;
	string FirstString; FirstString += FirstChar;
	string CompareString; CompareString += CompareChar;
	string const RegExp("*}*"); // Exist EndChar inside a String?

// first: Search for a '$' - Sign.
	//string copy(path);
	string result1; //(copy);    // for split-calls
	string result0 = split(path, result1, CompareChar);
	while (!result0.empty()) {
		string copy1(result0); // contains String after $
		
		// Check, if there is an EndChar inside original String.
		
		if (!regexMatch(copy1, RegExp)) {
			// No EndChar inside. So we are finished
			result1 += CompareString + result0;
			result0.clear();
			continue;
		}

		string res1;
		string res0 = split(copy1, res1, EndChar);
		// Now res1 holds the environmentvariable
		// First, check, if Contents is ok.
		if (res1.empty()) { // No environmentvariable. Continue Loop.
			result1 += CompareString + FirstString;
			result0  = res0;
			continue;
		}
		// check contents of res1
		char const * res1_contents = res1.c_str();
		if (*res1_contents != FirstChar) {
			// Again No Environmentvariable
			result1 += CompareString;
			result0 = res0;
		}

		// Check for variable names
		// Situation ${} is detected as "No Environmentvariable"
		char const * cp1 = res1_contents + 1;
		bool result = isalpha(*cp1) || (*cp1 == UnderscoreChar);
		++cp1;
		while (*cp1 && result) {
			result = isalnum(*cp1) || 
				(*cp1 == UnderscoreChar); 
			++cp1;
		}

		if (!result) {
			// no correct variable name
			result1 += CompareString + res1 + EndString;
			result0  = split(res0, res1, CompareChar);
			result1 += res1;
			continue;
		}
            
		string env = GetEnv(res1_contents+1);
		if (!env.empty()) {
			// Congratulations. Environmentvariable found
			result1 += env;
		} else {
			result1 += CompareString + res1 + EndString;
		}
		// Next $-Sign?
		result0  = split(res0, res1, CompareChar);
		result1 += res1;
	} 
	return result1;
}  // ReplaceEnvironmentPath


// Make relative path out of two absolute paths
string MakeRelPath(string const & abspath0, string const & basepath0)
// Makes relative path out of absolute path. If it is deeper than basepath,
// it's easy. If basepath and abspath share something (they are all deeper
// than some directory), it'll be rendered using ..'s. If they are completely
// different, then the absolute path will be used as relative path.
{
	// This is a hack. It should probaly be done in another way. Lgb.
	string abspath = CleanupPath(abspath0);
	string basepath = CleanupPath(basepath0);
	if (abspath.empty())
		return "<unknown_path>";

	const int abslen = abspath.length();
	const int baselen = basepath.length();
	
	// Find first different character
	int i = 0;
	while (i < abslen && i < baselen && abspath[i] == basepath[i]) ++i;

	// Go back to last /
	if (i < abslen && i < baselen
	    || (i<abslen && abspath[i] != '/' && i == baselen)
	    || (i<baselen && basepath[i] != '/' && i == abslen))
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
	string buf;
	int j = i;
	while (j < baselen) {
		if (basepath[j] == '/') {
			if (j + 1 == baselen) break;
			buf += "../";
		}
		++j;
	}

	// Append relative stuff from common directory to abspath
	if (abspath[i] == '/') ++i;
	for (; i < abslen; ++i)
		buf += abspath[i];
	// Remove trailing /
	if (suffixIs(buf, '/'))
		buf.erase(buf.length() - 1);
	// Substitute empty with .
	if (buf.empty())
		buf = '.';
	return buf;
}


// Append sub-directory(ies) to a path in an intelligent way
string AddPath(string const & path, string const & path_2)
{
	string buf;
	string path2 = CleanupPath(path_2);

	if (!path.empty() && path != "." && path != "./") {
		buf = CleanupPath(path);
		if (path[path.length() - 1] != '/')
			buf += '/';
	}

	if (!path2.empty()){
	        int p2start = path2.find_first_not_of('/');

		int p2end = path2.find_last_not_of('/');

		string tmp = path2.substr(p2start, p2end - p2start + 1);
		buf += tmp + '/';
	}
	return buf;
}


/* 
 Change extension of oldname to extension.
 Strips path off if no_path == true.
 If no extension on oldname, just appends.
 */
string ChangeExtension(string const & oldname, string const & extension, 
			bool no_path) 
{
	string::size_type last_slash = oldname.rfind('/');
	string::size_type last_dot = oldname.rfind('.');
	if (last_dot < last_slash && last_slash != string::npos)
		last_dot = string::npos;
	
	string ext;
	// Make sure the extension starts with a dot
	if (!extension.empty() && extension[0] != '.')
		ext= '.' + extension;
	else
		ext = extension;
	string ret_str;
	if (no_path && last_slash != string::npos) {
		++last_slash; // step it
		ret_str = oldname.substr(last_slash,
					 last_dot - last_slash) + ext;
	} else
		ret_str = oldname.substr(0, last_dot) + ext;
	return CleanupPath(ret_str);
}


// Creates a nice compact path for displaying
string MakeDisplayPath (string const & path, unsigned int threshold)
{
	const int l1 = path.length();

	// First, we try a relative path compared to home
	string home = GetEnvPath("HOME");
	string relhome = MakeRelPath(path, home);

	unsigned int l2 = relhome.length();

	string prefix;

	// If we backup from home or don't have a relative path,
	// this try is no good
	if (prefixIs(relhome, "../") || AbsolutePath(relhome)) {
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
		
		string temp;
		
		while (relhome.length() > threshold)
			relhome = split(relhome, temp, '/');

		// Did we shortend everything away?
		if (relhome.empty()) {
			// Yes, filename in itself is too long.
			// Pick the start and the end of the filename.
			relhome = OnlyFilename(path);
			string head = relhome.substr(0, threshold/2 - 3);

			l2 = relhome.length();
			string tail =
				relhome.substr(l2 - threshold/2 - 2, l2 - 1);
			relhome = head + "..." + tail;
		}
	}
	return prefix + relhome;
}


bool LyXReadLink(string const & File, string & Link)
{
	char LinkBuffer[512];
	// Should be PATH_MAX but that needs autconf support
	int nRead = readlink(File.c_str(), LinkBuffer, sizeof(LinkBuffer)-1);
	if (nRead <= 0)
		return false;
	LinkBuffer[nRead] = 0;
	Link = LinkBuffer;
	return true;
}


typedef pair<int, string> cmdret;
static
cmdret do_popen(string const & cmd)
{
	// One question is if we should use popen or
	// create our own popen based on fork, exec, pipe
	// of course the best would be to have a
	// pstream (process stream), with the
	// variants ipstream, opstream
	FILE * inf = popen(cmd.c_str(), "r");
	string ret;
	int c = fgetc(inf);
	while (c != EOF) {
		ret += static_cast<char>(c);
		c = fgetc(inf);
	}
	int pret = pclose(inf);
	return make_pair(pret, ret);
}


string findtexfile(string const & fil, string const & /*format*/)
{
	/* There is no problem to extend this function too use other
	   methods to look for files. It could be setup to look
	   in environment paths and also if wanted as a last resort
	   to a recursive find. One of the easier extensions would
	   perhaps be to use the LyX file lookup methods. But! I am
	   going to implement this until I see some demand for it.
	   Lgb
	*/
	
	// If the file can be found directly, we just return a
	// absolute path version of it. 
        if (FileInfo(fil).exist())
		return MakeAbsPath(fil);

        // No we try to find it using kpsewhich.
	// It seems from the kpsewhich manual page that it is safe to use
	// kpsewhich without --format: "When the --format option is not
	// given, the search path used when looking for a file is inferred
	// from the name given, by looking for a known extension. If no
	// known extension is found, the search path for TeX source files
	// is used."
	// However, we want to take advantage of the format sine almost all
	// the different formats has environment variables that can be used
	// to controll which paths to search. f.ex. bib looks in
	// BIBINPUTS and TEXBIB. Small list follows:
	// bib - BIBINPUTS, TEXBIB
	// bst - BSTINPUTS
	// graphic/figure - TEXPICTS, TEXINPUTS
	// ist - TEXINDEXSTYLE, INDEXSTYLE
	// pk - PROGRAMFONTS, PKFONTS, TEXPKS, GLYPHFONTS, TEXFONTS
	// tex - TEXINPUTS
	// tfm - TFMFONTS, TEXFONTS
	// This means that to use kpsewhich in the best possible way we
	// should help it by setting additional path in the approp. envir.var.
        string kpsecmd = "kpsewhich " + fil;

        cmdret c = do_popen(kpsecmd);
	
        lyxerr[Debug::LATEX] << "kpse status = " << c.first << "\n"
			     << "kpse result = `" << strip(c.second, '\n') 
			     << "'" << endl;
        return c.first != -1 ? strip(c.second, '\n') : string();
}
