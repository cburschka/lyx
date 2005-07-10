/**
 * \file filetools.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * parts Copyright 1985, 1990, 1993 Free Software Foundation, Inc.
 *
 * \author Ivan Schreter
 * \author Dirk Niggemann
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 *
 * General path-mangling functions
 */

#include <config.h>

#include "support/convert.h"
#include "support/environment.h"
#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/package.h"
#include "support/path.h"
#include "support/systemcall.h"

// FIXME Interface violation
#include "gettext.h"
#include "debug.h"

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

#include <fcntl.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <utility>
#include <fstream>
#include <sstream>

#ifndef CXX_GLOBAL_CSTD
using std::fgetc;
using std::isalnum;
using std::isalpha;
#endif

using std::endl;
using std::getline;
using std::make_pair;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::vector;

namespace fs = boost::filesystem;

namespace lyx {
namespace support {

bool IsLyXFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".lyx");
}


bool IsSGMLFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".sgml");
}


string const latex_path(string const & original_path,
		latex_path_extension extension,
		latex_path_dots dots)
{
	string path = subst(original_path, "\\", "/");
	path = subst(path, "~", "\\string~");
	if (path.find(' ') != string::npos)
		// We can't use '"' because " is sometimes active (e.g. if
		// babel is loaded with the "german" option)
		if (extension == EXCLUDE_EXTENSION) {
			string const base = ChangeExtension(path, string());
			string const ext = GetExtension(path);
			// ChangeExtension calls os::internal_path internally
			// so don't use it to re-add the extension.
			path = "\\string\"" + base + "\\string\"." + ext;
		} else
			path = "\\string\"" + path + "\\string\"";
	if (dots == ESCAPE_DOTS)
		return subst(path, ".", "\\lyxdot ");
	return path;
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


string const QuoteName(string const & name)
{
	return (os::shell() == os::UNIX) ?
		'\'' + name + '\'':
		'"' + name + '"';
}


// Is a file readable ?
bool IsFileReadable(string const & path)
{
	return fs::exists(path) && !fs::is_directory(path) && fs::is_readable(path);
}


//returns true: dir writeable
//	  false: not writeable
bool IsDirWriteable(string const & path)
{
	lyxerr[Debug::FILES] << "IsDirWriteable: " << path << endl;

	string const tmpfl(tempName(path, "lyxwritetest"));

	if (tmpfl.empty())
		return false;

	unlink(tmpfl);
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
		path_element = os::internal_path(path_element);
		if (!suffixIs(path_element, '/'))
			path_element+= '/';
		path_element = subst(path_element, "$$LyX",
				     package().system_support());
		path_element = subst(path_element, "$$User",
				     package().user_support());

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
	// EXCEPTIONS FIXME. Rewrite needed when we turn on exceptions. (Lgb)
	vector<string> dirlist;

	if (!(fs::exists(dir) && fs::is_directory(dir))) {
		lyxerr[Debug::FILES]
			<< "Directory \"" << dir
			<< "\" does not exist to DirList." << endl;
		return dirlist;
	}

	string extension;
	if (!ext.empty() && ext[0] != '.')
		extension += '.';
	extension += ext;

	fs::directory_iterator dit(dir);
	fs::directory_iterator end;
	for (; dit != end; ++dit) {
		string const & fil = dit->leaf();
		if (suffixIs(fil, extension)) {
			dirlist.push_back(fil);
		}
	}
	return dirlist;
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
	string fullname = FileSearch(AddPath(package().user_support(), dir),
				     name, ext);
	if (!fullname.empty())
		return fullname;

	if (!package().build_support().empty())
		fullname = FileSearch(AddPath(package().build_support(), dir),
				      name, ext);
	if (!fullname.empty())
		return fullname;

	return FileSearch(AddPath(package().system_support(), dir), name, ext);
}


string const
i18nLibFileSearch(string const & dir, string const & name,
		  string const & ext)
{
	// the following comments are from intl/dcigettext.c. We try
	// to mimick this behaviour here.
	/* The highest priority value is the `LANGUAGE' environment
	   variable. But we don't use the value if the currently
	   selected locale is the C locale. This is a GNU extension. */
	/* [Otherwise] We have to proceed with the POSIX methods of
	   looking to `LC_ALL', `LC_xxx', and `LANG'. */

	string lang = getEnv("LC_ALL");
	if (lang.empty()) {
		lang = getEnv("LC_MESSAGES");
		if (lang.empty()) {
			lang = getEnv("LANG");
			if (lang.empty())
				lang = "C";
		}
	}

	string const language = getEnv("LANGUAGE");
	if (lang != "C" && lang != "POSIX" && !language.empty())
		lang = language;

	string l;
	lang = split(lang, l, ':');
	while (!l.empty() && l != "C" && l != "POSIX") {
		string const tmp = LibFileSearch(dir,
						 token(l, '_', 0) + '_' + name,
						 ext);
		if (!tmp.empty())
			return tmp;
		lang = split(lang, l, ':');
	}

	return LibFileSearch(dir, name, ext);
}


string const LibScriptSearch(string const & command_in)
{
	string const token_scriptpath("$$s/");

	string command = command_in;
	// Find the starting position of "$$s/"
	string::size_type const pos1 = command.find(token_scriptpath);
	if (pos1 == string::npos)
		return command;
	// Find the end of the "$$s/some_subdir/some_script" word within
	// command. Assumes that the script name does not contain spaces.
	string::size_type const start_script = pos1 + 4;
	string::size_type const pos2 = command.find(' ', start_script);
	string::size_type const size_script = pos2 == string::npos?
		(command.size() - start_script) : pos2 - start_script;

	// Does this script file exist?
	string const script =
		LibFileSearch(".", command.substr(start_script, size_script));

	if (script.empty()) {
		// Replace "$$s/" with ""
		command.erase(pos1, 4);
	} else {
		// Replace "$$s/foo/some_script" with "<path to>/some_script".
		string::size_type const size_replace = size_script + 4;
		command.replace(pos1, size_replace, QuoteName(script));
	}

	return command;
}


namespace {

string const createTmpDir(string const & tempdir, string const & mask)
{
	lyxerr[Debug::FILES]
		<< "createTmpDir: tempdir=`" << tempdir << "'\n"
		<< "createTmpDir:    mask=`" << mask << '\'' << endl;

	string const tmpfl(tempName(tempdir, mask));
	// lyx::tempName actually creates a file to make sure that it
	// stays unique. So we have to delete it before we can create
	// a dir with the same name. Note also that we are not thread
	// safe because of the gap between unlink and mkdir. (Lgb)
	unlink(tmpfl);

	if (tmpfl.empty() || mkdir(tmpfl, 0700)) {
		lyxerr << "LyX could not create the temporary directory '"
		       << tmpfl << "'" << endl;
		return string();
	}

	return MakeAbsPath(tmpfl);
}

} // namespace anon


bool destroyDir(string const & tmpdir)
{

#ifdef __EMX__
	Path p(user_lyxdir());
#endif
	return fs::remove_all(tmpdir) > 0;
}


string const createBufferTmpDir()
{
	static int count;
	// We are in our own directory.  Why bother to mangle name?
	// In fact I wrote this code to circumvent a problematic behaviour
	// (bug?) of EMX mkstemp().
	string const tmpfl =
		package().temp_dir() + "/lyx_tmpbuf" +
		convert<string>(count++);

	if (mkdir(tmpfl, 0777)) {
		lyxerr << "LyX could not create the temporary directory '"
		       << tmpfl << "'" << endl;
		return string();
	}
	return tmpfl;
}


string const createLyXTmpDir(string const & deflt)
{
	if (!deflt.empty() && deflt != "/tmp") {
		if (mkdir(deflt, 0777)) {
#ifdef __EMX__
			Path p(package().user_support());
#endif
			if (IsDirWriteable(deflt)) {
				// deflt could not be created because it
				// did exist already, so let's create our own
				// dir inside deflt.
				return createTmpDir(deflt, "lyx_tmpdir");
			} else {
				// some other error occured.
				return createTmpDir("/tmp", "lyx_tmpdir");
			}
		} else
			return deflt;
	} else {
#ifdef __EMX__
		Path p(package().user_support());
#endif
		return createTmpDir("/tmp", "lyx_tmpdir");
	}
}


bool createDirectory(string const & path, int permission)
{
	string temp(rtrim(os::internal_path(path), "/"));

	BOOST_ASSERT(!temp.empty());

	if (mkdir(temp, permission))
		return false;

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
	string TempRel(os::internal_path(RelPath));
	// Since TempRel is NOT absolute, we can safely replace "//" with "/"
	TempRel = subst(TempRel, "//", "/");

	string TempBase;

	if (os::is_absolute_path(BasePath))
		TempBase = BasePath;
	else
		TempBase = AddPath(getcwd(), BasePath);

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
	return os::internal_path(TempBase);
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
		buf = os::internal_path(path);
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
		return getcwd() + '/' + RTemp;
	}
	if (Temp == "~") {
		return package().home_dir() + '/' + RTemp;
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
// Also converts paths like /foo//bar ==> /foo/bar
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

	// Normalise paths like /foo//bar ==> /foo/bar
	boost::RegEx regex("/{2,}");
	RTemp = regex.Merge(RTemp, "/");

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
	if (fs::exists(fname)) {
		ifstream ifs(fname.c_str());
		ostringstream ofs;
		if (ifs && ofs) {
			ofs << ifs.rdbuf();
			ifs.close();
			return ofs.str();
		}
	}
	lyxerr << "LyX was not able to read file '" << fname << '\'' << endl;
	return string();
}


// Search the string for ${VAR} and $VAR and replace VAR using getenv.
string const ReplaceEnvironmentPath(string const & path)
{
	// ${VAR} is defined as
	// $\{[A-Za-z_][A-Za-z_0-9]*\}
	static string const envvar_br = "[$]\\{([A-Za-z_][A-Za-z_0-9]*)\\}";

	// $VAR is defined as:
	// $\{[A-Za-z_][A-Za-z_0-9]*\}
	static string const envvar = "[$]([A-Za-z_][A-Za-z_0-9]*)";

	static boost::regex envvar_br_re("(.*)" + envvar_br + "(.*)");
	static boost::regex envvar_re("(.*)" + envvar + "(.*)");
	boost::smatch what;

	string result = path;
	while (1) {
		regex_match(result, what, envvar_br_re);
		if (!what[0].matched) {
			regex_match(result, what, envvar_re);
			if (!what[0].matched)
				break;
		}
		result = what.str(1) + getEnv(what.str(2)) + what.str(3);
	}
	return result;
}


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
	string const path2 = os::internal_path(path_2);

	if (!path.empty() && path != "." && path != "./") {
		buf = os::internal_path(path);
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
		ext= '.' + extension;
	else
		ext = extension;

	return os::internal_path(oldname.substr(0, last_dot) + ext);
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
// GZIP	\037\213	http://www.ietf.org/rfc/rfc1952.txt
// ZIP	PK...			http://www.halyava.ru/document/ind_arch.htm
// Z	\037\235		UNIX compress

string const getFormatFromContents(string const & filename)
{
	// paranoia check
	if (filename.empty() || !IsFileReadable(filename))
		return string();

	ifstream ifs(filename.c_str());
	if (!ifs)
		// Couldn't open file...
		return string();

	// gnuzip
	string const gzipStamp = "\037\213";

	// PKZIP
	string const zipStamp = "PK";

	// compress
	string const compressStamp = "\037\235";

	// Maximum strings to read
	int const max_count = 50;
	int count = 0;

	string str;
	string format;
	bool firstLine = true;
	while ((count++ < max_count) && format.empty()) {
		if (ifs.eof()) {
			lyxerr[Debug::GRAPHICS]
				<< "filetools(getFormatFromContents)\n"
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
		lyxerr[Debug::GRAPHICS]
			<< "Recognised Fileformat: " << format << endl;
		return format;
	}

	lyxerr[Debug::GRAPHICS]
		<< "filetools(getFormatFromContents)\n"
		<< "\tCouldn't find a known format!\n";
	return string();
}


/// check for zipped file
bool zippedFile(string const & name)
{
	string const type = getFormatFromContents(name);
	if (contains("gzip zip compress", type) && !type.empty())
		return true;
	return false;
}


string const unzippedFileName(string const & zipped_file)
{
	string const ext = GetExtension(zipped_file);
	if (ext == "gz" || ext == "z" || ext == "Z")
		return ChangeExtension(zipped_file, string());
	return "unzipped_" + zipped_file;
}


string const unzipFile(string const & zipped_file, string const & unzipped_file)
{
	string const tempfile = unzipped_file.empty() ?
		unzippedFileName(zipped_file) : unzipped_file;
	// Run gunzip
	string const command = "gunzip -c " + zipped_file + " > " + tempfile;
	Systemcall one;
	one.startscript(Systemcall::Wait, command);
	// test that command was executed successfully (anon)
	// yes, please do. (Lgb)
	return tempfile;
}


string const MakeDisplayPath(string const & path, unsigned int threshold)
{
	string str = path;

	string const home(package().home_dir());

	// replace /home/blah with ~/
	if (prefixIs(str, home))
		str = subst(str, home, "~");

	if (str.length() <= threshold)
		return os::external_path(str);

	string const prefix = ".../";
	string temp;

	while (str.length() > threshold)
		str = split(str, temp, '/');

	// Did we shorten everything away?
	if (str.empty()) {
		// Yes, filename itself is too long.
		// Pick the start and the end of the filename.
		str = OnlyFilename(path);
		string const head = str.substr(0, threshold / 2 - 3);

		string::size_type len = str.length();
		string const tail =
			str.substr(len - threshold / 2 - 2, len - 1);
		str = head + "..." + tail;
	}

	return os::external_path(prefix + str);
}


bool LyXReadLink(string const & file, string & link, bool resolve)
{
#ifdef HAVE_READLINK
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
#else
	return false;
#endif
}


cmd_ret const RunCommand(string const & cmd)
{
	// FIXME: replace all calls to RunCommand with ForkedCall
	// (if the output is not needed) or the code in ispell.C
	// (if the output is needed).

	// One question is if we should use popen or
	// create our own popen based on fork, exec, pipe
	// of course the best would be to have a
	// pstream (process stream), with the
	// variants ipstream, opstream

#if defined (HAVE_POPEN)
	FILE * inf = ::popen(cmd.c_str(), os::popen_read_mode());
#elif defined (HAVE__POPEN)
	FILE * inf = ::_popen(cmd.c_str(), os::popen_read_mode());
#else
#error No popen() function.
#endif

	// (Claus Hentschel) Check if popen was succesful ;-)
	if (!inf) {
		return make_pair(-1, string());
		lyxerr << "RunCommand:: could not start child process" << endl;
		}

	string ret;
	int c = fgetc(inf);
	while (c != EOF) {
		ret += static_cast<char>(c);
		c = fgetc(inf);
	}

#if defined (HAVE_PCLOSE)
	int const pret = pclose(inf);
#elif defined (HAVE__PCLOSE)
	int const pret = _pclose(inf);
#else
#error No pclose() function.
#endif

	if (pret == -1)
		perror("RunCommand:: could not terminate child process");

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
	if (fs::exists(fil))
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

	lyxerr[Debug::LATEX] << "kpse status = " << c.first << '\n'
		 << "kpse result = `" << rtrim(c.second, "\n")
		 << '\'' << endl;
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
	if (fs::exists(a))
		unlink(a);
}


void readBB_lyxerrMessage(string const & file, bool & zipped,
	string const & message)
{
	lyxerr[Debug::GRAPHICS] << "[readBB_from_PSFile] "
		<< message << std::endl;
#ifdef WITH_WARNINGS
#warning Why is this func deleting a file? (Lgb)
#endif
	if (zipped)
		unlink(file);
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
	string const format = getFormatFromContents(file_);

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


int compare_timestamps(string const & file1, string const & file2)
{
	BOOST_ASSERT(AbsolutePath(file1) && AbsolutePath(file2));

	// If the original is newer than the copy, then copy the original
	// to the new directory.

	int cmp = 0;
	if (fs::exists(file1) && fs::exists(file2)) {
		double const tmp = difftime(fs::last_write_time(file1),
					    fs::last_write_time(file2));
		if (tmp != 0)
			cmp = tmp > 0 ? 1 : -1;

	} else if (fs::exists(file1)) {
		cmp = 1;
	} else if (fs::exists(file2)) {
		cmp = -1;
	}

	return cmp;
}

} //namespace support
} // namespace lyx
