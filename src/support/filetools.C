/*
	filetools.C (former paths.C) - part of LyX project
	General path-mangling functions
	Copyright 1996 Ivan Schreter
	Parts Copyright 1996 Dirk Niggemann
	Parts Copyright 1985, 1990, 1993 Free Software Foundation, Inc.
	Parts Copyright 1996 Asger Alstrup

	See also filetools.h.

	lyx-filetool.C : tools functions for file/path handling
	this file is part of LyX, the High Level Word Processor
	Copyright 1995-1996, Matthias Ettrich and the LyX Team

*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation "filetools.h"
#endif

#include "debug.h"
#include "support/lstrings.h"
#include "support/systemcall.h"

#include "filetools.h"
#include "lstrings.h"
#include "frontends/Alert.h"
#include "FileInfo.h"
#include "support/path.h"        // I know it's OS/2 specific (SMiyata)
#include "gettext.h"
#include "lyxlib.h"
#include "os.h"

#include "Lsstream.h"

#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>

#include <utility>
#include <fstream>


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

#ifndef CXX_GLOBAL_CSTD
using std::fgetc;
using std::isalpha;
using std::isalnum;
#endif

using std::make_pair;
using std::pair;
using std::endl;
using std::ifstream;
using std::vector;
using std::getline;

extern string system_lyxdir;
extern string build_lyxdir;
extern string user_lyxdir;


bool IsLyXFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".lyx");
}


bool IsSGMLFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".sgml");
}


// Substitutes spaces with underscores in filename (and path)
string const MakeLatexName(string const & file)
{
	string name = OnlyFilename(file);
	string const path = OnlyPath(file);

	for (string::size_type i = 0; i < name.length(); ++i) {
		name[i] &= 0x7f; // set 8th bit to 0
	};

	// ok so we scan through the string twice, but who cares.
	string const keep("abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"@!\"'()*+,-./0123456789:;<=>?[]`|");

	string::size_type pos = 0;
	while ((pos = name.find_first_not_of(keep, pos)) != string::npos) {
		name[pos++] = '_';
	}
	return AddName(path, name);
}


// Substitutes spaces with underscores in filename (and path)
string const QuoteName(string const & name)
{
	return (os::shell() == os::UNIX) ?
		"\'" + name + "\'":
		"\"" + name + "\"";
}


// Is a file readable ?
bool IsFileReadable(string const & path)
{
	FileInfo file(path);
	return file.isOK() && file.isRegular() && file.readable();
}


// Is a file read_only?
// return 1 read-write
//	  0 read_only
//	 -1 error (doesn't exist, no access, anything else)
int IsFileWriteable(string const & path)
{
	FileInfo fi(path);

	if (fi.access(FileInfo::wperm|FileInfo::rperm)) // read-write
		return 1;
	if (fi.readable()) // read-only
		return 0;
	return -1; // everything else.
}


//returns true: dir writeable
//	  false: not writeable
bool IsDirWriteable(string const & path)
{
	lyxerr[Debug::FILES] << "IsDirWriteable: " << path << endl;

	string const tmpfl(lyx::tempName(path, "lyxwritetest"));

	if (tmpfl.empty())
		return false;

	lyx::unlink(tmpfl);
	return true;
}


// Uses a string of paths separated by ";"s to find a file to open.
// Can't cope with pathnames with a ';' in them. Returns full path to file.
// If path entry begins with $$LyX/, use system_lyxdir
// If path entry begins with $$User/, use user_lyxdir
// Example: "$$User/doc;$$LyX/doc"
string const FileOpenSearch(string const & path, string const & name,
			     string const & ext)
{
	string real_file;
	string path_element;
	bool notfound = true;
	string tmppath = split(path, path_element, ';');

	while (notfound && !path_element.empty()) {
		path_element = os::slashify_path(path_element);
		if (!suffixIs(path_element, '/'))
			path_element+= '/';
		path_element = subst(path_element, "$$LyX", system_lyxdir);
		path_element = subst(path_element, "$$User", user_lyxdir);

		real_file = FileSearch(path_element, name, ext);

		if (real_file.empty()) {
			do {
				tmppath = split(tmppath, path_element, ';');
			} while (!tmppath.empty() && path_element.empty());
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


/// Returns a vector of all files in directory dir having extension ext.
vector<string> const DirList(string const & dir, string const & ext)
{
	// This is a non-error checking C/system implementation
	string extension(ext);
	if (!extension.empty() && extension[0] != '.')
		extension.insert(0, ".");
	vector<string> dirlist;
	DIR * dirp = ::opendir(dir.c_str());
	if (!dirp) {
		lyxerr[Debug::FILES]
			<< "Directory \"" << dir
			<< "\" does not exist to DirList." << endl;
		return dirlist;
	}

	dirent * dire;
	while ((dire = ::readdir(dirp))) {
		string const fil = dire->d_name;
		if (suffixIs(fil, extension)) {
			dirlist.push_back(fil);
		}
	}
	::closedir(dirp);
	return dirlist;
	/* I would have prefered to take a vector<string>& as parameter so
	   that we could avoid the copy of the vector when returning.
	   Then we would use:
	   dirlist.swap(argvec);
	   to avoid the copy. (Lgb)
	*/
	/* A C++ implementaion will look like this:
	   string extension(ext);
	   if (extension[0] != '.') extension.insert(0, ".");
	   vector<string> dirlist;
	   directory_iterator dit("dir");
	   while (dit != directory_iterator()) {
		   string fil = dit->filename;
		   if (prefixIs(fil, extension)) {
			   dirlist.push_back(fil);
		   }
		   ++dit;
	   }
	   dirlist.swap(argvec);
	   return;
	*/
}


// Returns the real name of file name in directory path, with optional
// extension ext.
string const FileSearch(string const & path, string const & name,
			string const & ext)
{
	// if `name' is an absolute path, we ignore the setting of `path'
	// Expand Environmentvariables in 'name'
	string const tmpname = ReplaceEnvironmentPath(name);
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
string const LibFileSearch(string const & dir, string const & name,
			   string const & ext)
{
	string fullname = FileSearch(AddPath(user_lyxdir, dir), name, ext);
	if (!fullname.empty())
		return fullname;

	if (!build_lyxdir.empty())
		fullname = FileSearch(AddPath(build_lyxdir, dir), name, ext);
	if (!fullname.empty())
		return fullname;

	return FileSearch(AddPath(system_lyxdir, dir), name, ext);
}


string const
i18nLibFileSearch(string const & dir, string const & name,
		  string const & ext)
{
	// this comment is from intl/dcigettext.c. We try to mimick this
	// behaviour here.
	/* The highest priority value is the `LANGUAGE' environment
	   variable. But we don't use the value if the currently
	   selected locale is the C locale. This is a GNU extension. */

	string const lc_all = GetEnv("LC_ALL");
	string lang = GetEnv("LANGUAGE");
	if (lang.empty() || lc_all == "C") {
		lang = lc_all;
		if (lang.empty()) {
			lang = GetEnv("LANG");
		}
	}

	lang = token(lang, '_', 0);

	if (lang.empty() || lang == "C")
		return LibFileSearch(dir, name, ext);
	else {
		string const tmp = LibFileSearch(dir, lang + '_' + name,
						 ext);
		if (!tmp.empty())
			return tmp;
		else
			return LibFileSearch(dir, name, ext);
	}
}


string const LibScriptSearch(string const & command)
{
	string script;
	string args = command;
	args = split(args, script, ' ');
	script = LibFileSearch("scripts", script);
	if (script.empty())
		return command;
	else if (args.empty())
		return script;
	else
		return script + ' ' + args;
}


string const GetEnv(string const & envname)
{
	// f.ex. what about error checking?
	char const * const ch = getenv(envname.c_str());
	string const envstr = !ch ? "" : ch;
	return envstr;
}


string const GetEnvPath(string const & name)
{
#ifndef __EMX__
	string const pathlist = subst(GetEnv(name), ':', ';');
#else
	string const pathlist = os::slashify_path(GetEnv(name));
#endif
	return rtrim(pathlist, ";");
}


bool PutEnv(string const & envstr)
{
	// CHECK Look at and fix this.
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
	int const retval = lyx::putenv(leaker);

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
	string const str = envstr.split(varname,'=');
	int const retval = ::setenv(varname.c_str(), str.c_str(), true);
#else
	// No environment setting function. Can this happen?
	int const retval = 1; //return an error condition.
#endif
#endif
	return retval == 0;
}


bool PutEnvPath(string const & envstr)
{
	return PutEnv(envstr);
}


namespace {

int DeleteAllFilesInDir(string const & path)
{
	// I have decided that we will be using parts from the boost
	// library. Check out http://www.boost.org/
	// For directory access we will then use the directory_iterator.
	// Then the code will be something like:
	// directory_iterator dit(path);
	// directory_iterator dend;
	// if (dit == dend) {
	//         Alert::err_alert(_("Error! Cannot open directory:"), path);
	//         return -1;
	// }
	// for (; dit != dend; ++dit) {
	//         string filename(*dit);
	//         if (filename == "." || filename == "..")
	//                 continue;
	//         string unlinkpath(AddName(path, filename));
	//         if (lyx::unlink(unlinkpath))
	//                 Alert::err_alert(_("Error! Could not remove file:"),
	//                              unlinkpath);
	// }
	// return 0;
	DIR * dir = ::opendir(path.c_str());
	if (!dir) {
		Alert::err_alert (_("Error! Cannot open directory:"), path);
		return -1;
	}
	struct dirent * de;
	int return_value = 0;
	while ((de = readdir(dir))) {
		string const temp = de->d_name;
		if (temp == "." || temp == "..")
			continue;
		string const unlinkpath = AddName (path, temp);

		lyxerr[Debug::FILES] << "Deleting file: " << unlinkpath
				     << endl;

		bool deleted = true;
		FileInfo fi(unlinkpath);
		if (fi.isOK() && fi.isDir())
			deleted = (DeleteAllFilesInDir(unlinkpath) == 0);
		deleted &= (lyx::unlink(unlinkpath) == 0);
		if (!deleted) {
			Alert::err_alert(_("Error! Could not remove file:"),
				unlinkpath);
			return_value = -1;
		}
	}
	closedir(dir);
	return return_value;
}


string const CreateTmpDir(string const & tempdir, string const & mask)
{
	lyxerr[Debug::FILES]
		<< "CreateTmpDir: tempdir=`" << tempdir << "'\n"
		<< "CreateTmpDir:    mask=`" << mask << "'" << endl;

	string const tmpfl(lyx::tempName(tempdir, mask));
	// lyx::tempName actually creates a file to make sure that it
	// stays unique. So we have to delete it before we can create
	// a dir with the same name. Note also that we are not thread
	// safe because of the gap between unlink and mkdir. (Lgb)
	lyx::unlink(tmpfl.c_str());

	if (tmpfl.empty() || lyx::mkdir(tmpfl, 0700)) {
		Alert::err_alert(_("Error! Couldn't create temporary directory:"),
			     tempdir);
		return string();
	}
	return MakeAbsPath(tmpfl);
}


int DestroyTmpDir(string const & tmpdir, bool Allfiles)
{
#ifdef __EMX__
	Path p(user_lyxdir);
#endif
	if (Allfiles && DeleteAllFilesInDir(tmpdir)) {
		return -1;
	}
	if (lyx::rmdir(tmpdir)) {
		Alert::err_alert(_("Error! Couldn't delete temporary directory:"),
			     tmpdir);
		return -1;
	}
	return 0;
}

} // namespace anon


string const CreateBufferTmpDir(string const & pathfor)
{
	static int count;
	static string const tmpdir(pathfor.empty() ? os::getTmpDir() : pathfor);
	// We are in our own directory.  Why bother to mangle name?
	// In fact I wrote this code to circumvent a problematic behaviour (bug?)
	// of EMX mkstemp().
	string const tmpfl = tmpdir + "/lyx_tmpbuf" + tostr(count++);
	if (lyx::mkdir(tmpfl, 0777)) {
		Alert::err_alert(_("Error! Couldn't create temporary directory:"),
			     tmpdir);
		return string();
	}
	return tmpfl;
}


int DestroyBufferTmpDir(string const & tmpdir)
{
	return DestroyTmpDir(tmpdir, true);
}


string const CreateLyXTmpDir(string const & deflt)
{
	if ((!deflt.empty()) && (deflt  != "/tmp")) {
		if (lyx::mkdir(deflt, 0777)) {
#ifdef __EMX__
		Path p(user_lyxdir);
#endif
			return CreateTmpDir(deflt, "lyx_tmpdir");
		} else
			return deflt;
	} else {
#ifdef __EMX__
		Path p(user_lyxdir);
#endif
		return CreateTmpDir("/tmp", "lyx_tmpdir");
	}
}


// FIXME: no need for separate method like this ...
int DestroyLyXTmpDir(string const & tmpdir)
{
	return DestroyTmpDir(tmpdir, true);
}


// Creates directory. Returns true if succesfull
bool createDirectory(string const & path, int permission)
{
	string temp(rtrim(os::slashify_path(path), "/"));

	if (temp.empty()) {
		Alert::alert(_("Internal error!"),
			   _("Call to createDirectory with invalid name"));
		return false;
	}

	if (lyx::mkdir(temp, permission)) {
		Alert::err_alert (_("Error! Couldn't create directory:"), temp);
		return false;
	}
	return true;
}


// Strip filename from path name
string const OnlyPath(string const & Filename)
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
string const MakeAbsPath(string const & RelPath, string const & BasePath)
{
	// checks for already absolute path
	if (os::is_absolute_path(RelPath))
		return RelPath;

	// Copies given paths
	string TempRel(os::slashify_path(RelPath));
	// Since TempRel is NOT absolute, we can safely replace "//" with "/"
	TempRel = subst(TempRel, "//", "/");

	string TempBase;

	if (os::is_absolute_path(BasePath))
		TempBase = BasePath;
	else
		TempBase = AddPath(lyx::getcwd(), BasePath);

	// Handle /./ at the end of the path
	while (suffixIs(TempBase, "/./"))
		TempBase.erase(TempBase.length() - 2);

	// processes relative path
	string RTemp(TempRel);
	string Temp;

	while (!RTemp.empty()) {
		// Split by next /
		RTemp = split(RTemp, Temp, '/');

		if (Temp == ".") continue;
		if (Temp == "..") {
			// Remove one level of TempBase
			string::difference_type i = TempBase.length() - 2;
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
		} else if (Temp.empty() && !RTemp.empty()) {
				TempBase = os::current_root() + RTemp;
				RTemp.erase();
		} else {
			// Add this piece to TempBase
			if (!suffixIs(TempBase, '/'))
				TempBase += '/';
			TempBase += Temp;
		}
	}

	// returns absolute path
	return os::slashify_path(TempBase);
}


// Correctly append filename to the pathname.
// If pathname is '.', then don't use pathname.
// Chops any path of filename.
string const AddName(string const & path, string const & fname)
{
	// Get basename
	string const basename(OnlyFilename(fname));

	string buf;

	if (path != "." && path != "./" && !path.empty()) {
		buf = os::slashify_path(path);
		if (!suffixIs(path, '/'))
			buf += '/';
	}

	return buf + basename;
}


// Strips path from filename
string const OnlyFilename(string const & fname)
{
	if (fname.empty())
		return fname;

	string::size_type j = fname.rfind('/');
	if (j == string::npos) // no '/' in fname
		return fname;

	// Strip to basename
	return fname.substr(j + 1);
}


/// Returns true is path is absolute
bool AbsolutePath(string const & path)
{
	return os::is_absolute_path(path);
}



// Create absolute path. If impossible, don't do anything
// Supports ./ and ~/. Later we can add support for ~logname/. (Asger)
string const ExpandPath(string const & path)
{
	// checks for already absolute path
	string RTemp(ReplaceEnvironmentPath(path));
	if (os::is_absolute_path(RTemp))
		return RTemp;

	string Temp;
	string const copy(RTemp);

	// Split by next /
	RTemp = split(RTemp, Temp, '/');

	if (Temp == ".") {
		return lyx::getcwd() /*GetCWD()*/ + '/' + RTemp;
	}
	if (Temp == "~") {
		return GetEnvPath("HOME") + '/' + RTemp;
	}
	if (Temp == "..") {
		return MakeAbsPath(copy);
	}
	// Don't know how to handle this
	return copy;
}


// Normalize a path
// Constracts path/../path
// Can't handle "../../" or "/../" (Asger)
string const NormalizePath(string const & path)
{
	string TempBase;
	string RTemp;
	string Temp;

	if (os::is_absolute_path(path))
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
			string::difference_type i = TempBase.length() - 2;
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


string const GetFileContents(string const & fname)
{
	FileInfo finfo(fname);
	if (finfo.exist()) {
		ifstream ifs(fname.c_str());
		ostringstream ofs;
		if (ifs && ofs) {
			ofs << ifs.rdbuf();
			ifs.close();
			return ofs.str().c_str();
		}
	}
	lyxerr << "LyX was not able to read file '" << fname << "'" << endl;
	return string();
}


//
// Search ${...} as Variable-Name inside the string and replace it with
// the denoted environmentvariable
// Allow Variables according to
//  variable :=  '$' '{' [A-Za-z_]{[A-Za-z_0-9]*} '}'
//

string const ReplaceEnvironmentPath(string const & path)
{
//
// CompareChar: Environmentvariables starts with this character
// PathChar:    Next path component start with this character
// while CompareChar found do:
//       Split String with PathChar
//	 Search Environmentvariable
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
			result0.erase();
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

		string env(GetEnv(res1_contents + 1));
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
string const MakeRelPath(string const & abspath, string const & basepath)
// Makes relative path out of absolute path. If it is deeper than basepath,
// it's easy. If basepath and abspath share something (they are all deeper
// than some directory), it'll be rendered using ..'s. If they are completely
// different, then the absolute path will be used as relative path.
{
	string::size_type const abslen = abspath.length();
	string::size_type const baselen = basepath.length();

	string::size_type i = os::common_path(abspath, basepath);

	if (i == 0) {
		// actually no match - cannot make it relative
		return abspath;
	}

	// Count how many dirs there are in basepath above match
	// and append as many '..''s into relpath
	string buf;
	string::size_type j = i;
	while (j < baselen) {
		if (basepath[j] == '/') {
			if (j + 1 == baselen)
				break;
			buf += "../";
		}
		++j;
	}

	// Append relative stuff from common directory to abspath
	if (abspath[i] == '/')
		++i;
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
string const AddPath(string const & path, string const & path_2)
{
	string buf;
	string const path2 = os::slashify_path(path_2);

	if (!path.empty() && path != "." && path != "./") {
		buf = os::slashify_path(path);
		if (path[path.length() - 1] != '/')
			buf += '/';
	}

	if (!path2.empty()) {
		string::size_type const p2start = path2.find_first_not_of('/');
		string::size_type const p2end = path2.find_last_not_of('/');
		string const tmp = path2.substr(p2start, p2end - p2start + 1);
		buf += tmp + '/';
	}
	return buf;
}


/*
 Change extension of oldname to extension.
 Strips path off if no_path == true.
 If no extension on oldname, just appends.
 */
string const ChangeExtension(string const & oldname, string const & extension)
{
	string::size_type const last_slash = oldname.rfind('/');
	string::size_type last_dot = oldname.rfind('.');
	if (last_dot < last_slash && last_slash != string::npos)
		last_dot = string::npos;

	string ext;
	// Make sure the extension starts with a dot
	if (!extension.empty() && extension[0] != '.')
		ext= "." + extension;
	else
		ext = extension;

	return os::slashify_path(oldname.substr(0, last_dot) + ext);
}


/// Return the extension of the file (not including the .)
string const GetExtension(string const & name)
{
	string::size_type const last_slash = name.rfind('/');
	string::size_type const last_dot = name.rfind('.');
	if (last_dot != string::npos &&
	    (last_slash == string::npos || last_dot > last_slash))
		return name.substr(last_dot + 1,
				   name.length() - (last_dot + 1));
	else
		return string();
}

// the different filetypes and what they contain in one of the first lines
// (dots are any characters).		(Herbert 20020131)
// AGR	Grace...
// BMP	BM...
// EPS	%!PS-Adobe-3.0 EPSF...
// EPSI like EPS and with
//      %%BeginPreview...
// FIG	#FIG...
// FITS ...BITPIX...
// GIF	GIF...
// JPG	JFIF
// PDF	%PDF-...
// PNG	.PNG...
// PBM	P1... or P4	(B/W)
// PGM	P2... or P5	(Grayscale)
// PPM	P3... or P6	(color)
// PS	%!PS-Adobe-2.0 or 1.0,  no "EPSF"!
// SGI	\001\332...	(decimal 474)
// TGIF	%TGIF...
// TIFF	II... or MM...
// XBM	..._bits[]...
// XPM	/* XPM */    sometimes missing (f.ex. tgif-export)
//      ...static char *...
// XWD	\000\000\000\151	(0x00006900) decimal 105
//
// GZIP	\037\213\010\010...	http://www.ietf.org/rfc/rfc1952.txt
// ZIP	PK...			http://www.halyava.ru/document/ind_arch.htm
// Z	\037\177		UNIX compress

/// return the "extension" which belongs to the contents.
/// for no knowing contents return the extension. Without
/// an extension and unknown contents we return "user"
string const getExtFromContents(string const & filename)
{
	// paranoia check
	if (filename.empty() || !IsFileReadable(filename))
		return string();


	ifstream ifs(filename.c_str());
	if (!ifs)
		// Couldn't open file...
		return string();

	// gnuzip
	string const gzipStamp = "\037\213\010\010";

	// PKZIP
	string const zipStamp = "PK";

	// compress
	string const compressStamp = "\037\177";

	// Maximum strings to read
	int const max_count = 50;
	int count = 0;

	string str, format;
	bool firstLine = true;
	while ((count++ < max_count) && format.empty()) {
		if (ifs.eof()) {
			lyxerr[Debug::GRAPHICS]
				<< "filetools(getExtFromContents)\n"
				<< "\tFile type not recognised before EOF!"
				<< endl;
			break;
		}

		getline(ifs, str);
		string const stamp = str.substr(0,2);
		if (firstLine && str.size() >= 2) {
			// at first we check for a zipped file, because this
			// information is saved in the first bytes of the file!
			// also some graphic formats which save the information
			// in the first line, too.
			if (prefixIs(str, gzipStamp)) {
				format =  "gzip";

			} else if (stamp == zipStamp) {
				format =  "zip";

			} else if (stamp == compressStamp) {
				format =  "compress";

			// the graphics part
			} else if (stamp == "BM") {
				format =  "bmp";

			} else if (stamp == "\001\332") {
				format =  "sgi";

			// PBM family
			// Don't need to use str.at(0), str.at(1) because
			// we already know that str.size() >= 2
			} else if (str[0] == 'P') {
				switch (str[1]) {
				case '1':
				case '4':
					format =  "pbm";
				    break;
				case '2':
				case '5':
					format =  "pgm";
				    break;
				case '3':
				case '6':
					format =  "ppm";
				}
				break;

			} else if ((stamp == "II") || (stamp == "MM")) {
				format =  "tiff";

			} else if (prefixIs(str,"%TGIF")) {
				format =  "tgif";

			} else if (prefixIs(str,"#FIG")) {
				format =  "fig";

			} else if (prefixIs(str,"GIF")) {
				format =  "gif";

			} else if (str.size() > 3) {
				int const c = ((str[0] << 24) & (str[1] << 16) &
					       (str[2] << 8)  & str[3]);
				if (c == 105) {
					format =  "xwd";
				}
			}

			firstLine = false;
		}

		if (!format.empty())
		    break;
		else if (contains(str,"EPSF"))
			// dummy, if we have wrong file description like
			// %!PS-Adobe-2.0EPSF"
			format =  "eps";

		else if (contains(str,"Grace"))
			format =  "agr";

		else if (contains(str,"JFIF"))
			format =  "jpg";

		else if (contains(str,"%PDF"))
			format =  "pdf";

		else if (contains(str,"PNG"))
			format =  "png";

		else if (contains(str,"%!PS-Adobe")) {
			// eps or ps
			ifs >> str;
			if (contains(str,"EPSF"))
				format = "eps";
			else
			    format = "ps";
		}

		else if (contains(str,"_bits[]"))
			format = "xbm";

		else if (contains(str,"XPM") || contains(str, "static char *"))
			format = "xpm";

		else if (contains(str,"BITPIX"))
			format = "fits";
	}

	if (!format.empty()) {
		// if we have eps than epsi is also possible
		// we have to check for a preview
		if (format == "eps") {
			lyxerr[Debug::GRAPHICS]
				<< "\teps detected -> test for an epsi ..."
				<< endl;
			while (count++ < max_count) {
				if (ifs.eof())
					break;
				getline(ifs, str);
				if (contains(str, "BeginPreview")) {
					format = "epsi";
					count = max_count;
				}
			}
		}
		lyxerr[Debug::GRAPHICS]
			<< "Recognised Fileformat: " << format << endl;
		return format;
	}

	string const ext(GetExtension(filename));
	lyxerr[Debug::GRAPHICS]
		<< "filetools(getExtFromContents)\n"
		<< "\tCouldn't find a known Type!\n";
	if (!ext.empty()) {
	    lyxerr[Debug::GRAPHICS]
		<< "\twill take the file extension -> "
		<< ext << endl;
		return ext;
	} else {
	    lyxerr[Debug::GRAPHICS]
		<< "\twill use ext or a \"user\" defined format" << endl;
	    return "user";
	}
}


/// check for zipped file
bool zippedFile(string const & name)
{
	string const type = getExtFromContents(name);
	if (contains("gzip zip compress", type) && !type.empty())
		return true;
	return false;
}


string const unzipFile(string const & zipped_file)
{
	string const file = ChangeExtension(zipped_file, string());
	string  const tempfile = lyx::tempName(string(), file);
	// Run gunzip
	string const command = "gunzip -c " + zipped_file + " > " + tempfile;
	Systemcall one;
	one.startscript(Systemcall::Wait, command);
	// test that command was executed successfully (anon)
	// yes, please do. (Lgb)
	return tempfile;
}


// Creates a nice compact path for displaying
string const
MakeDisplayPath (string const & path, unsigned int threshold)
{
	string::size_type const l1 = path.length();

	// First, we try a relative path compared to home
	string const home(GetEnvPath("HOME"));
	string relhome = MakeRelPath(path, home);

	string::size_type l2 = relhome.length();

	string prefix;

	// If we backup from home or don't have a relative path,
	// this try is no good
	if (prefixIs(relhome, "../") || os::is_absolute_path(relhome)) {
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
			string const head = relhome.substr(0, threshold/2 - 3);

			l2 = relhome.length();
			string const tail =
				relhome.substr(l2 - threshold/2 - 2, l2 - 1);
			relhome = head + "..." + tail;
		}
	}
	return prefix + relhome;
}


bool LyXReadLink(string const & file, string & link, bool resolve)
{
	char linkbuffer[512];
	// Should be PATH_MAX but that needs autconf support
	int const nRead = ::readlink(file.c_str(),
				     linkbuffer, sizeof(linkbuffer) - 1);
	if (nRead <= 0)
		return false;
	linkbuffer[nRead] = '\0'; // terminator
	if (resolve)
		link = MakeAbsPath(linkbuffer, OnlyPath(file));
	else
		link = linkbuffer;
	return true;
}


cmd_ret const RunCommand(string const & cmd)
{
	// One question is if we should use popen or
	// create our own popen based on fork, exec, pipe
	// of course the best would be to have a
	// pstream (process stream), with the
	// variants ipstream, opstream

	FILE * inf = ::popen(cmd.c_str(), os::popen_read_mode());

	// (Claus Hentschel) Check if popen was succesful ;-)
	if (!inf)
		return make_pair(-1, string());

	string ret;
	int c = fgetc(inf);
	while (c != EOF) {
		ret += static_cast<char>(c);
		c = fgetc(inf);
	}
	int const pret = pclose(inf);
	return make_pair(pret, ret);
}


string const findtexfile(string const & fil, string const & /*format*/)
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
	string const kpsecmd = "kpsewhich " + fil;

	cmd_ret const c = RunCommand(kpsecmd);

	lyxerr[Debug::LATEX] << "kpse status = " << c.first << "\n"
		 << "kpse result = `" << rtrim(c.second, "\n")
		 << "'" << endl;
	if (c.first != -1)
		return os::internal_path(rtrim(c.second, "\n\r"));
	else
		return string();
}


void removeAutosaveFile(string const & filename)
{
	string a = OnlyPath(filename);
	a += '#';
	a += OnlyFilename(filename);
	a += '#';
	FileInfo const fileinfo(a);
	if (fileinfo.exist()) {
		if (lyx::unlink(a) != 0) {
			Alert::err_alert(_("Could not delete auto-save file!"), a);
		}
	}
}


void readBB_lyxerrMessage(string const & file, bool & zipped,
	string const & message)
{
	lyxerr[Debug::GRAPHICS] << "[readBB_from_PSFile] "
		<< message << std::endl;
	if (zipped)
		lyx::unlink(file);
}


string const readBB_from_PSFile(string const & file)
{
	// in a (e)ps-file it's an entry like %%BoundingBox:23 45 321 345
	// It seems that every command in the header has an own line,
	// getline() should work for all files.
	// On the other hand some plot programs write the bb at the
	// end of the file. Than we have in the header:
	// %%BoundingBox: (atend)
	// In this case we must check the end.
	bool zipped = zippedFile(file);
	string const file_ = zipped ?
		string(unzipFile(file)) : string(file);
	string const format = getExtFromContents(file_);

	if (format != "eps" && format != "ps") {
		readBB_lyxerrMessage(file_, zipped,"no(e)ps-format");
		return string();
	}

	std::ifstream is(file_.c_str());
	while (is) {
		string s;
		getline(is,s);
		if (contains(s,"%%BoundingBox:") && !contains(s,"atend")) {
			string const bb = ltrim(s.substr(14));
			readBB_lyxerrMessage(file_, zipped, bb);
			return bb;
		}
	}
	readBB_lyxerrMessage(file_, zipped, "no bb found");
	return string();
}
