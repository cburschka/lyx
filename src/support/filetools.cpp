/**
 * \file filetools.cpp
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
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

// FIXME Interface violation
#include "gettext.h"
#include "debug.h"

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

#include <fcntl.h>

#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <utility>
#include <fstream>
#include <sstream>

#ifndef CXX_GLOBAL_CSTD
using std::fgetc;
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

bool isLyXFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".lyx");
}


bool isSGMLFilename(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".sgml");
}


bool isValidLaTeXFilename(string const & filename)
{
	string const invalid_chars("#$%{}()[]\"^");
	if (filename.find_first_of(invalid_chars) != string::npos)
		return false;
	else
		return true;
}


string const latex_path(string const & original_path,
		latex_path_extension extension,
		latex_path_dots dots)
{
	// On cygwin, we may need windows or posix style paths.
	string path = os::latex_path(original_path);
	path = subst(path, "~", "\\string~");
	if (path.find(' ') != string::npos) {
		// We can't use '"' because " is sometimes active (e.g. if
		// babel is loaded with the "german" option)
		if (extension == EXCLUDE_EXTENSION) {
			// ChangeExtension calls os::internal_path internally
			// so don't use it to remove the extension.
			string const ext = getExtension(path);
			string const base = ext.empty() ?
				path :
				path.substr(0, path.length() - ext.length() - 1);
			// ChangeExtension calls os::internal_path internally
			// so don't use it to re-add the extension.
			path = "\\string\"" + base + "\\string\"." + ext;
		} else {
			path = "\\string\"" + path + "\\string\"";
		}
	}

	return dots == ESCAPE_DOTS ? subst(path, ".", "\\lyxdot ") : path;
}


// Substitutes spaces with underscores in filename (and path)
string const makeLatexName(string const & file)
{
	string name = onlyFilename(file);
	string const path = onlyPath(file);

	// ok so we scan through the string twice, but who cares.
	string const keep = "abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"@!'()*+,-./0123456789:;<=>?[]`|";

	string::size_type pos = 0;
	while ((pos = name.find_first_not_of(keep, pos)) != string::npos)
		name[pos++] = '_';

	return addName(path, name);
}


string const quoteName(string const & name, quote_style style)
{
	switch(style) {
	case quote_shell:
		// This does not work for filenames containing " (windows)
		// or ' (all other OSes). This can't be changed easily, since
		// we would need to adapt the command line parser in
		// Forkedcall::generateChild. Therefore we don't pass user
		// filenames to child processes if possible. We store them in
		// a python script instead, where we don't have these
		// limitations.
		return (os::shell() == os::UNIX) ?
			'\'' + name + '\'':
			'"' + name + '"';
	case quote_python:
		return "\"" + subst(subst(name, "\\", "\\\\"), "\"", "\\\"")
		     + "\"";
	}
	// shut up stupid compiler
	return string();
}


bool isFileReadable(FileName const & filename)
{
	std::string const path = filename.toFilesystemEncoding();
	try {
		// it seems that fs::exists can throw an exception
		// when the file is not readable.
		return fs::exists(path) 
			&& !fs::is_directory(path) 
			&& fs::is_readable(path);
	} catch (fs::filesystem_error const & fe){
		lyxerr << "isFileReadable() error with path: "
			<< path << endl;
		lyxerr << fe.what() << endl;
		return false;
	}
}


bool doesFileExist(FileName const & filename)
{
	std::string const path = filename.toFilesystemEncoding();
	try {
		// fs::exists can throw an exception
		// f.ex. if the drive was unmounted.
		return fs::exists(path);
	} catch (fs::filesystem_error const & fe){
		lyxerr << "doesFileExist() error with path: "
			<< path << endl;
		lyxerr << fe.what() << endl;
		return false;
	}
}


//returns true: dir writeable
//	  false: not writeable
bool isDirWriteable(FileName const & path)
{
	LYXERR(Debug::FILES) << "isDirWriteable: " << path << endl;

	FileName const tmpfl(tempName(path, "lyxwritetest"));

	if (tmpfl.empty())
		return false;

	unlink(tmpfl);
	return true;
}


#if 0
// Uses a string of paths separated by ";"s to find a file to open.
// Can't cope with pathnames with a ';' in them. Returns full path to file.
// If path entry begins with $$LyX/, use system_lyxdir
// If path entry begins with $$User/, use user_lyxdir
// Example: "$$User/doc;$$LyX/doc"
FileName const fileOpenSearch(string const & path, string const & name,
			     string const & ext)
{
	FileName real_file;
	string path_element;
	bool notfound = true;
	string tmppath = split(path, path_element, ';');

	while (notfound && !path_element.empty()) {
		path_element = os::internal_path(path_element);
		if (!suffixIs(path_element, '/'))
			path_element += '/';
		path_element = subst(path_element, "$$LyX",
				     package().system_support().absFilename());
		path_element = subst(path_element, "$$User",
				     package().user_support().absFilename());

		real_file = fileSearch(path_element, name, ext);

		if (real_file.empty()) {
			do {
				tmppath = split(tmppath, path_element, ';');
			} while (!tmppath.empty() && path_element.empty());
		} else {
			notfound = false;
		}
	}
	return real_file;
}
#endif


/// Returns a vector of all files in directory dir having extension ext.
vector<FileName> const dirList(FileName const & dir, string const & ext)
{
	// EXCEPTIONS FIXME. Rewrite needed when we turn on exceptions. (Lgb)
	vector<FileName> dirlist;

	string const encoded_dir = dir.toFilesystemEncoding();
	if (!(doesFileExist(dir) && fs::is_directory(encoded_dir))) {
		LYXERR(Debug::FILES)
			<< "Directory \"" << dir
			<< "\" does not exist to DirList." << endl;
		return dirlist;
	}

	string extension;
	if (!ext.empty() && ext[0] != '.')
		extension += '.';
	extension += ext;

	fs::directory_iterator dit(encoded_dir);
	fs::directory_iterator end;
	for (; dit != end; ++dit) {
		string const & fil = dit->leaf();
		if (suffixIs(fil, extension))
			dirlist.push_back(FileName::fromFilesystemEncoding(
					encoded_dir + '/' + fil));
	}
	return dirlist;
}


// Returns the real name of file name in directory path, with optional
// extension ext.
FileName const fileSearch(string const & path, string const & name,
			  string const & ext, search_mode mode)
{
	// if `name' is an absolute path, we ignore the setting of `path'
	// Expand Environmentvariables in 'name'
	string const tmpname = replaceEnvironmentPath(name);
	FileName fullname(makeAbsPath(tmpname, path));
	// search first without extension, then with it.
	if (isFileReadable(fullname))
		return fullname;
	if (ext.empty())
		// We are done.
		return mode == allow_unreadable ? fullname : FileName();
	// Only add the extension if it is not already the extension of
	// fullname.
	if (getExtension(fullname.absFilename()) != ext)
		fullname = FileName(addExtension(fullname.absFilename(), ext));
	if (isFileReadable(fullname) || mode == allow_unreadable)
		return fullname;
	return FileName();
}


// Search the file name.ext in the subdirectory dir of
//   1) user_lyxdir
//   2) build_lyxdir (if not empty)
//   3) system_lyxdir
FileName const libFileSearch(string const & dir, string const & name,
			   string const & ext)
{
	FileName fullname = fileSearch(addPath(package().user_support().absFilename(), dir),
				     name, ext);
	if (!fullname.empty())
		return fullname;

	if (!package().build_support().empty())
		fullname = fileSearch(addPath(package().build_support().absFilename(), dir),
				      name, ext);
	if (!fullname.empty())
		return fullname;

	return fileSearch(addPath(package().system_support().absFilename(), dir), name, ext);
}


FileName const i18nLibFileSearch(string const & dir, string const & name,
		  string const & ext)
{
	/* The highest priority value is the `LANGUAGE' environment
	   variable. But we don't use the value if the currently
	   selected locale is the C locale. This is a GNU extension.

	   Otherwise, we use a trick to guess what gettext has done:
	   each po file is able to tell us its name. (JMarc)
	*/

	string lang = to_ascii(_("[[Replace with the code of your language]]"));
	string const language = getEnv("LANGUAGE");
	if (!lang.empty() && !language.empty())
		lang = language;

	string l;
	lang = split(lang, l, ':');
	while (!l.empty()) {
		FileName tmp;
		// First try with the full name
		tmp = libFileSearch(addPath(dir, l), name, ext);
		if (!tmp.empty())
			return tmp;

		// Then the name without country code
		string const shortl = token(l, '_', 0);
		if (shortl != l) {
			tmp = libFileSearch(addPath(dir, shortl), name, ext);
			if (!tmp.empty())
				return tmp;
		}

#if 1
		// For compatibility, to be removed later (JMarc)
		tmp = libFileSearch(dir, token(l, '_', 0) + '_' + name,
				    ext);
		if (!tmp.empty()) {
			lyxerr << "i18nLibFileSearch: File `" << tmp
			       << "' has been found by the old method" <<endl;
			return tmp;
		}
#endif
		lang = split(lang, l, ':');
	}

	return libFileSearch(dir, name, ext);
}


string const libScriptSearch(string const & command_in, quote_style style)
{
	static string const token_scriptpath = "$$s/";

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
		libFileSearch(".", command.substr(start_script, size_script)).absFilename();

	if (script.empty()) {
		// Replace "$$s/" with ""
		command.erase(pos1, 4);
	} else {
		// Replace "$$s/foo/some_script" with "<path to>/some_script".
		string::size_type const size_replace = size_script + 4;
		command.replace(pos1, size_replace, quoteName(script, style));
	}

	return command;
}


namespace {

FileName const createTmpDir(FileName const & tempdir, string const & mask)
{
	LYXERR(Debug::FILES)
		<< "createTmpDir: tempdir=`" << tempdir << "'\n"
		<< "createTmpDir:    mask=`" << mask << '\'' << endl;

	FileName const tmpfl(tempName(tempdir, mask));
	// lyx::tempName actually creates a file to make sure that it
	// stays unique. So we have to delete it before we can create
	// a dir with the same name. Note also that we are not thread
	// safe because of the gap between unlink and mkdir. (Lgb)
	unlink(tmpfl);

	if (tmpfl.empty() || mkdir(tmpfl, 0700)) {
		lyxerr << "LyX could not create the temporary directory '"
		       << tmpfl << "'" << endl;
		return FileName();
	}

	return tmpfl;
}

} // namespace anon


bool destroyDir(FileName const & tmpdir)
{
	try {
		return fs::remove_all(tmpdir.toFilesystemEncoding()) > 0;
	} catch (fs::filesystem_error const & fe){
		lyxerr << "Could not delete " << tmpdir << ". (" << fe.what() << ")" << std::endl;
		return false;
	}
}


string const createBufferTmpDir()
{
	static int count;
	// We are in our own directory.  Why bother to mangle name?
	// In fact I wrote this code to circumvent a problematic behaviour
	// (bug?) of EMX mkstemp().
	string const tmpfl =
		package().temp_dir().absFilename() + "/lyx_tmpbuf" +
		convert<string>(count++);

	if (mkdir(FileName(tmpfl), 0777)) {
		lyxerr << "LyX could not create the temporary directory '"
		       << tmpfl << "'" << endl;
		return string();
	}
	return tmpfl;
}


FileName const createLyXTmpDir(FileName const & deflt)
{
	if (!deflt.empty() && deflt.absFilename() != "/tmp") {
		if (mkdir(deflt, 0777)) {
			if (isDirWriteable(deflt)) {
				// deflt could not be created because it
				// did exist already, so let's create our own
				// dir inside deflt.
				return createTmpDir(deflt, "lyx_tmpdir");
			} else {
				// some other error occured.
				return createTmpDir(FileName("/tmp"), "lyx_tmpdir");
			}
		} else
			return deflt;
	} else {
		return createTmpDir(FileName("/tmp"), "lyx_tmpdir");
	}
}


bool createDirectory(FileName const & path, int permission)
{
	BOOST_ASSERT(!path.empty());
	return mkdir(path, permission) == 0;
}


// Strip filename from path name
string const onlyPath(string const & filename)
{
	// If empty filename, return empty
	if (filename.empty())
		return filename;

	// Find last / or start of filename
	string::size_type j = filename.rfind('/');
	return j == string::npos ? "./" : filename.substr(0, j + 1);
}


// Convert relative path into absolute path based on a basepath.
// If relpath is absolute, just use that.
// If basepath is empty, use CWD as base.
FileName const makeAbsPath(string const & relPath, string const & basePath)
{
	// checks for already absolute path
	if (os::is_absolute_path(relPath))
		return FileName(relPath);

	// Copies given paths
	string tempRel = os::internal_path(relPath);
	// Since TempRel is NOT absolute, we can safely replace "//" with "/"
	tempRel = subst(tempRel, "//", "/");

	string tempBase;

	if (os::is_absolute_path(basePath))
		tempBase = basePath;
	else
		tempBase = addPath(getcwd().absFilename(), basePath);

	// Handle /./ at the end of the path
	while (suffixIs(tempBase, "/./"))
		tempBase.erase(tempBase.length() - 2);

	// processes relative path
	string rTemp = tempRel;
	string temp;

	while (!rTemp.empty()) {
		// Split by next /
		rTemp = split(rTemp, temp, '/');

		if (temp == ".") continue;
		if (temp == "..") {
			// Remove one level of TempBase
			string::difference_type i = tempBase.length() - 2;
			if (i < 0)
				i = 0;
			while (i > 0 && tempBase[i] != '/')
				--i;
			if (i > 0)
				tempBase.erase(i, string::npos);
			else
				tempBase += '/';
		} else if (temp.empty() && !rTemp.empty()) {
				tempBase = os::current_root() + rTemp;
				rTemp.erase();
		} else {
			// Add this piece to TempBase
			if (!suffixIs(tempBase, '/'))
				tempBase += '/';
			tempBase += temp;
		}
	}

	// returns absolute path
	return FileName(os::internal_path(tempBase));
}


// Correctly append filename to the pathname.
// If pathname is '.', then don't use pathname.
// Chops any path of filename.
string const addName(string const & path, string const & fname)
{
	string const basename = onlyFilename(fname);
	string buf;

	if (path != "." && path != "./" && !path.empty()) {
		buf = os::internal_path(path);
		if (!suffixIs(path, '/'))
			buf += '/';
	}

	return buf + basename;
}


// Strips path from filename
string const onlyFilename(string const & fname)
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
bool absolutePath(string const & path)
{
	return os::is_absolute_path(path);
}


// Create absolute path. If impossible, don't do anything
// Supports ./ and ~/. Later we can add support for ~logname/. (Asger)
string const expandPath(string const & path)
{
	// checks for already absolute path
	string rTemp = replaceEnvironmentPath(path);
	if (os::is_absolute_path(rTemp))
		return rTemp;

	string temp;
	string const copy = rTemp;

	// Split by next /
	rTemp = split(rTemp, temp, '/');

	if (temp == ".")
		return getcwd().absFilename() + '/' + rTemp;

	if (temp == "~")
		return package().home_dir().absFilename() + '/' + rTemp;

	if (temp == "..")
		return makeAbsPath(copy).absFilename();

	// Don't know how to handle this
	return copy;
}


// Normalize a path. Constracts path/../path
// Can't handle "../../" or "/../" (Asger)
// Also converts paths like /foo//bar ==> /foo/bar
string const normalizePath(string const & path)
{
	// Normalize paths like /foo//bar ==> /foo/bar
	static boost::regex regex("/{2,}");
	string const tmppath = boost::regex_merge(path, regex, "/");

	fs::path const npath = fs::path(tmppath, fs::no_check).normalize();

	if (!npath.is_complete())
		return "./" + npath.string() + '/';

	return npath.string() + '/';
}


string const getFileContents(FileName const & fname)
{
	string const encodedname = fname.toFilesystemEncoding();
	if (doesFileExist(fname)) {
		ifstream ifs(encodedname.c_str());
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
string const replaceEnvironmentPath(string const & path)
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
docstring const makeRelPath(docstring const & abspath, docstring const & basepath)
// Makes relative path out of absolute path. If it is deeper than basepath,
// it's easy. If basepath and abspath share something (they are all deeper
// than some directory), it'll be rendered using ..'s. If they are completely
// different, then the absolute path will be used as relative path.
{
	docstring::size_type const abslen = abspath.length();
	docstring::size_type const baselen = basepath.length();

	docstring::size_type i = os::common_path(abspath, basepath);

	if (i == 0) {
		// actually no match - cannot make it relative
		return abspath;
	}

	// Count how many dirs there are in basepath above match
	// and append as many '..''s into relpath
	docstring buf;
	docstring::size_type j = i;
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
string const addPath(string const & path, string const & path_2)
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


string const changeExtension(string const & oldname, string const & extension)
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


string const removeExtension(string const & name)
{
	return changeExtension(name, string());
}


string const addExtension(string const & name, string const & extension)
{
	if (!extension.empty() && extension[0] != '.')
		return name + '.' + extension;
	return name + extension;
}


/// Return the extension of the file (not including the .)
string const getExtension(string const & name)
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

string const getFormatFromContents(FileName const & filename)
{
	// paranoia check
	if (filename.empty() || !isFileReadable(filename))
		return string();

	ifstream ifs(filename.toFilesystemEncoding().c_str());
	if (!ifs)
		// Couldn't open file...
		return string();

	// gnuzip
	static string const gzipStamp = "\037\213";

	// PKZIP
	static string const zipStamp = "PK";

	// compress
	static string const compressStamp = "\037\235";

	// Maximum strings to read
	int const max_count = 50;
	int count = 0;

	string str;
	string format;
	bool firstLine = true;
	while ((count++ < max_count) && format.empty()) {
		if (ifs.eof()) {
			LYXERR(Debug::GRAPHICS)
				<< "filetools(getFormatFromContents)\n"
				<< "\tFile type not recognised before EOF!"
				<< endl;
			break;
		}

		getline(ifs, str);
		string const stamp = str.substr(0, 2);
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
		LYXERR(Debug::GRAPHICS)
			<< "Recognised Fileformat: " << format << endl;
		return format;
	}

	LYXERR(Debug::GRAPHICS)
		<< "filetools(getFormatFromContents)\n"
		<< "\tCouldn't find a known format!\n";
	return string();
}


/// check for zipped file
bool zippedFile(FileName const & name)
{
	string const type = getFormatFromContents(name);
	if (contains("gzip zip compress", type) && !type.empty())
		return true;
	return false;
}


string const unzippedFileName(string const & zipped_file)
{
	string const ext = getExtension(zipped_file);
	if (ext == "gz" || ext == "z" || ext == "Z")
		return changeExtension(zipped_file, string());
	return onlyPath(zipped_file) + "unzipped_" + onlyFilename(zipped_file);
}


FileName const unzipFile(FileName const & zipped_file, string const & unzipped_file)
{
	FileName const tempfile = FileName(unzipped_file.empty() ?
		unzippedFileName(zipped_file.toFilesystemEncoding()) :
		unzipped_file);
	// Run gunzip
	string const command = "gunzip -c " +
		zipped_file.toFilesystemEncoding() + " > " +
		tempfile.toFilesystemEncoding();
	Systemcall one;
	one.startscript(Systemcall::Wait, command);
	// test that command was executed successfully (anon)
	// yes, please do. (Lgb)
	return tempfile;
}


docstring const makeDisplayPath(string const & path, unsigned int threshold)
{
	string str = path;

	// If file is from LyXDir, display it as if it were relative.
	string const system = package().system_support().absFilename();
	if (prefixIs(str, system) && str != system)
		return from_utf8("[" + str.erase(0, system.length()) + "]");

	// replace /home/blah with ~/
	string const home = package().home_dir().absFilename();
	if (!home.empty() && prefixIs(str, home))
		str = subst(str, home, "~");

	if (str.length() <= threshold)
		return from_utf8(os::external_path(str));

	string const prefix = ".../";
	string temp;

	while (str.length() > threshold)
		str = split(str, temp, '/');

	// Did we shorten everything away?
	if (str.empty()) {
		// Yes, filename itself is too long.
		// Pick the start and the end of the filename.
		str = onlyFilename(path);
		string const head = str.substr(0, threshold / 2 - 3);

		string::size_type len = str.length();
		string const tail =
			str.substr(len - threshold / 2 - 2, len - 1);
		str = head + "..." + tail;
	}

	return from_utf8(os::external_path(prefix + str));
}


bool readLink(FileName const & file, FileName & link)
{
#ifdef HAVE_READLINK
	char linkbuffer[512];
	// Should be PATH_MAX but that needs autconf support
	string const encoded = file.toFilesystemEncoding();
	int const nRead = ::readlink(encoded.c_str(),
				     linkbuffer, sizeof(linkbuffer) - 1);
	if (nRead <= 0)
		return false;
	linkbuffer[nRead] = '\0'; // terminator
	link = makeAbsPath(linkbuffer, onlyPath(file.absFilename()));
	return true;
#else
	return false;
#endif
}


cmd_ret const runCommand(string const & cmd)
{
	// FIXME: replace all calls to RunCommand with ForkedCall
	// (if the output is not needed) or the code in ISpell.cpp
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
		lyxerr << "RunCommand:: could not start child process" << endl;
		return make_pair(-1, string());
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


FileName const findtexfile(string const & fil, string const & /*format*/)
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
	FileName const absfile(makeAbsPath(fil));
	if (doesFileExist(absfile))
		return absfile;

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

	cmd_ret const c = runCommand(kpsecmd);

	LYXERR(Debug::LATEX) << "kpse status = " << c.first << '\n'
		 << "kpse result = `" << rtrim(c.second, "\n\r")
		 << '\'' << endl;
	if (c.first != -1)
		return FileName(os::internal_path(rtrim(to_utf8(from_filesystem8bit(c.second)),
							"\n\r")));
	else
		return FileName();
}


void removeAutosaveFile(string const & filename)
{
	string a = onlyPath(filename);
	a += '#';
	a += onlyFilename(filename);
	a += '#';
	FileName const autosave(a);
	if (doesFileExist(autosave))
		unlink(autosave);
}


void readBB_lyxerrMessage(FileName const & file, bool & zipped,
	string const & message)
{
	LYXERR(Debug::GRAPHICS) << "[readBB_from_PSFile] "
		<< message << std::endl;
#ifdef WITH_WARNINGS
#warning Why is this func deleting a file? (Lgb)
#endif
	if (zipped)
		unlink(file);
}


string const readBB_from_PSFile(FileName const & file)
{
	// in a (e)ps-file it's an entry like %%BoundingBox:23 45 321 345
	// It seems that every command in the header has an own line,
	// getline() should work for all files.
	// On the other hand some plot programs write the bb at the
	// end of the file. Than we have in the header:
	// %%BoundingBox: (atend)
	// In this case we must check the end.
	bool zipped = zippedFile(file);
	FileName const file_ = zipped ? unzipFile(file) : file;
	string const format = getFormatFromContents(file_);

	if (format != "eps" && format != "ps") {
		readBB_lyxerrMessage(file_, zipped,"no(e)ps-format");
		return string();
	}

	static boost::regex bbox_re(
		"^%%BoundingBox:\\s*([[:digit:]]+)\\s+([[:digit:]]+)\\s+([[:digit:]]+)\\s+([[:digit:]]+)");
	std::ifstream is(file_.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		getline(is,s);
		boost::smatch what;
		if (regex_match(s, what, bbox_re)) {
			// Our callers expect the tokens in the string
			// separated by single spaces.
			// FIXME: change return type from string to something
			// sensible
			ostringstream os;
			os << what.str(1) << ' ' << what.str(2) << ' '
			   << what.str(3) << ' ' << what.str(4);
			string const bb = os.str();
			readBB_lyxerrMessage(file_, zipped, bb);
			return bb;
		}
	}
	readBB_lyxerrMessage(file_, zipped, "no bb found");
	return string();
}


int compare_timestamps(FileName const & filename1, FileName const & filename2)
{
	// If the original is newer than the copy, then copy the original
	// to the new directory.

	string const file1 = filename1.toFilesystemEncoding();
	string const file2 = filename2.toFilesystemEncoding();
	int cmp = 0;
	if (doesFileExist(filename1) && doesFileExist(filename2)) {
		double const tmp = difftime(fs::last_write_time(file1),
					    fs::last_write_time(file2));
		if (tmp != 0)
			cmp = tmp > 0 ? 1 : -1;

	} else if (doesFileExist(filename1)) {
		cmp = 1;
	} else if (doesFileExist(filename2)) {
		cmp = -1;
	}

	return cmp;
}

} //namespace support
} // namespace lyx
