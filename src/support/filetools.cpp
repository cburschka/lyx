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

#include "LyXRC.h"

#include "support/filetools.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Messages.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/qstring_helpers.h"

#include <QDir>

#include "support/lassert.h"
#include "support/regex.h"

#include <fcntl.h>

#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <utility>
#include <fstream>
#include <sstream>
#include <vector>

#if defined (_WIN32)
#include <io.h>
#include <windows.h>
#endif

using namespace std;

#define USE_QPROCESS

namespace lyx {
namespace support {

bool isLyXFileName(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".lyx");
}


bool isSGMLFileName(string const & filename)
{
	return suffixIs(ascii_lowercase(filename), ".sgml");
}


bool isValidLaTeXFileName(string const & filename)
{
	string const invalid_chars("#%\"");
	return filename.find_first_of(invalid_chars) == string::npos;
}


bool isValidDVIFileName(string const & filename)
{
	string const invalid_chars("${}()[]^");
	return filename.find_first_of(invalid_chars) == string::npos;
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
			// changeExtension calls os::internal_path internally
			// so don't use it to remove the extension.
			string const ext = getExtension(path);
			string const base = ext.empty() ?
				path :
				path.substr(0, path.length() - ext.length() - 1);
			// changeExtension calls os::internal_path internally
			// so don't use it to re-add the extension.
			path = "\\string\"" + base + "\\string\"." + ext;
		} else {
			path = "\\string\"" + path + "\\string\"";
		}
	}

	if (dots != ESCAPE_DOTS)
		return path;

	// Replace dots with the lyxdot macro, but only in the file name,
	// not the directory part.
	// addName etc call os::internal_path internally
	// so don't use them for path manipulation
	// The directory separator is always '/' for LaTeX.
	string::size_type pos = path.rfind('/');
	if (pos == string::npos)
		return subst(path, ".", "\\lyxdot ");
	return path.substr(0, pos) + subst(path.substr(pos), ".", "\\lyxdot ");
}


// Substitutes spaces with underscores in filename (and path)
FileName const makeLatexName(FileName const & file)
{
	string name = file.onlyFileName();
	string const path = file.onlyPath().absFileName() + "/";

	// ok so we scan through the string twice, but who cares.
	// FIXME: in Unicode time this will break for sure! There is
	// a non-latin world out there...
	string const keep = "abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"@!'()*+,-./0123456789:;<=>?[]`|";

	string::size_type pos = 0;
	while ((pos = name.find_first_not_of(keep, pos)) != string::npos)
		name[pos++] = '_';

	FileName latex_name(path + name);
	latex_name.changeExtension(".tex");
	return latex_name;
}


string const quoteName(string const & name, quote_style style)
{
	switch(style) {
	case quote_shell:
		// This does not work on native Windows for filenames
		// containing the following characters < > : " / \ | ? *
		// Moreover, it can't be made to work, as, according to
		// http://msdn.microsoft.com/en-us/library/aa365247(VS.85).aspx
		// those are reserved characters, and thus are forbidden.
		// Please, also note that the command-line parser in
		// ForkedCall::generateChild cannot deal with filenames
		// containing " or ', therefore we don't pass user filenames
		// to child processes if possible. We store them in a python
		// script instead, where we don't have these limitations.
#ifndef USE_QPROCESS
		return (os::shell() == os::UNIX) ?
			'\'' + subst(name, "'", "\'\\\'\'") + '\'' :
			'"' + name + '"';
#else
		// According to the QProcess parser, a single double
		// quote is represented by three consecutive ones.
		// Here we simply escape the double quote and let our
		// simple parser in Systemcall.cpp do the substitution.
		return '"' + subst(name, "\"", "\\\"") + '"';
#endif
	case quote_python:
		return "\"" + subst(subst(name, "\\", "\\\\"), "\"", "\\\"")
		     + "\"";
	}
	// shut up stupid compiler
	return string();
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
				     package().system_support().absFileName());
		path_element = subst(path_element, "$$User",
				     package().user_support().absFileName());

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
	if (fullname.isReadableFile())
		return fullname;
	if (ext.empty())
		// We are done.
		return mode == may_not_exist ? fullname : FileName();
	// Only add the extension if it is not already the extension of
	// fullname.
	if (getExtension(fullname.absFileName()) != ext)
		fullname = FileName(addExtension(fullname.absFileName(), ext));
	if (fullname.isReadableFile() || mode == may_not_exist)
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
	FileName fullname = fileSearch(addPath(package().user_support().absFileName(), dir),
				     name, ext);
	if (!fullname.empty())
		return fullname;

	if (!package().build_support().empty())
		fullname = fileSearch(addPath(package().build_support().absFileName(), dir),
				      name, ext);
	if (!fullname.empty())
		return fullname;

	return fileSearch(addPath(package().system_support().absFileName(), dir), name, ext);
}


FileName const i18nLibFileSearch(string const & dir, string const & name,
		  string const & ext)
{
	/* The highest priority value is the `LANGUAGE' environment
	   variable. But we don't use the value if the currently
	   selected locale is the C locale. This is a GNU extension.

	   Otherwise, w use a trick to guess what support/gettext.has done:
	   each po file is able to tell us its name. (JMarc)
	*/

	string lang = getGuiMessages().language();
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


FileName const imageLibFileSearch(string & dir, string const & name,
		  string const & ext)
{
	if (!lyx::lyxrc.icon_set.empty()) {
		string const imagedir = addPath(dir, lyx::lyxrc.icon_set);
		FileName const fn = libFileSearch(imagedir, name, ext);
		if (fn.exists()) {
			dir = imagedir;
			return fn;
		}
	}
	return libFileSearch(dir, name, ext);
}


string const commandPrep(string const & command_in)
{
	static string const token_scriptpath = "$$s/";
	string const python_call = "python -tt";

	string command = command_in;
	if (prefixIs(command_in, python_call))
		command = os::python() + command_in.substr(python_call.length());

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
		libFileSearch(".", command.substr(start_script, size_script)).absFileName();

	if (script.empty()) {
		// Replace "$$s/" with ""
		command.erase(pos1, 4);
	} else {
		quote_style style = quote_shell;
		if (prefixIs(command, os::python()))
			style = quote_python;

		// Replace "$$s/foo/some_script" with "<path to>/some_script".
		string::size_type const size_replace = size_script + 4;
		command.replace(pos1, size_replace, quoteName(script, style));
	}

	return command;
}


static FileName createTmpDir(FileName const & tempdir, string const & mask)
{
	LYXERR(Debug::FILES, "createTmpDir: tempdir=`" << tempdir << "'\n"
		<< "createTmpDir:    mask=`" << mask << '\'');

	FileName const tmpfl = FileName::tempName(tempdir, mask);

	if (tmpfl.empty() || !tmpfl.createDirectory(0700)) {
		LYXERR0("LyX could not create temporary directory in " << tempdir
			<< "'");
		return FileName();
	}

	return tmpfl;
}


FileName const createLyXTmpDir(FileName const & deflt)
{
	if (deflt.empty() || deflt == package().system_temp_dir())
		return createTmpDir(package().system_temp_dir(), "lyx_tmpdir");

	if (deflt.createDirectory(0777))
		return deflt;

	if (deflt.isDirWritable()) {
		// deflt could not be created because it
		// did exist already, so let's create our own
		// dir inside deflt.
		return createTmpDir(deflt, "lyx_tmpdir");
	} else {
		// some other error occured.
		return createTmpDir(package().system_temp_dir(), "lyx_tmpdir");
	}
}


// Strip filename from path name
string const onlyPath(string const & filename)
{
	// If empty filename, return empty
	if (filename.empty())
		return filename;

	// Find last / or start of filename
	size_t j = filename.rfind('/');
	return j == string::npos ? "./" : filename.substr(0, j + 1);
}


// Convert relative path into absolute path based on a basepath.
// If relpath is absolute, just use that.
// If basepath is empty, use CWD as base.
// Note that basePath can be a relative path, in the sense that it may
// not begin with "/" (e.g.), but it should NOT contain such constructs
// as "/../".
// FIXME It might be nice if the code didn't simply assume that.
FileName const makeAbsPath(string const & relPath, string const & basePath)
{
	// checks for already absolute path
	if (FileName::isAbsolute(relPath))
		return FileName(relPath);

	// Copies given paths
	string tempRel = os::internal_path(relPath);
	// Since TempRel is NOT absolute, we can safely replace "//" with "/"
	tempRel = subst(tempRel, "//", "/");

	string tempBase;

	if (FileName::isAbsolute(basePath))
		tempBase = basePath;
	else
		tempBase = addPath(FileName::getcwd().absFileName(), basePath);

	// Handle /./ at the end of the path
	while (suffixIs(tempBase, "/./"))
		tempBase.erase(tempBase.length() - 2);

	// processes relative path
	string rTemp = tempRel;
	string temp;

	// Check for a leading "~"
	// Split by first /
	rTemp = split(rTemp, temp, '/');
	if (temp == "~") {
		tempBase = Package::get_home_dir().absFileName();
		tempRel = rTemp;
	}

	rTemp = tempRel;
	while (!rTemp.empty()) {
		// Split by next /
		rTemp = split(rTemp, temp, '/');

		if (temp == ".") continue;
		if (temp == "..") {
			// Remove one level of TempBase
			if (tempBase.length() <= 1) {
				//this is supposed to be an absolute path, so...
				tempBase = "/";
				continue;
			}
			//erase a trailing slash if there is one
			if (suffixIs(tempBase, "/"))
				tempBase.erase(tempBase.length() - 1, string::npos);

			string::size_type i = tempBase.length() - 1;
			while (i > 0 && tempBase[i] != '/')
				--i;
			if (i > 0)
				tempBase.erase(i, string::npos);
			else
				tempBase = '/';
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
	return FileName(tempBase);
}


// Correctly append filename to the pathname.
// If pathname is '.', then don't use pathname.
// Chops any path of filename.
string const addName(string const & path, string const & fname)
{
	string const basename = onlyFileName(fname);
	string buf;

	if (path != "." && path != "./" && !path.empty()) {
		buf = os::internal_path(path);
		if (!suffixIs(path, '/'))
			buf += '/';
	}

	return buf + basename;
}


// Strips path from filename
string const onlyFileName(string const & fname)
{
	if (fname.empty())
		return fname;

	string::size_type j = fname.rfind('/');
	if (j == string::npos) // no '/' in fname
		return fname;

	// Strip to basename
	return fname.substr(j + 1);
}


// Create absolute path. If impossible, don't do anything
// Supports ./ and ~/. Later we can add support for ~logname/. (Asger)
string const expandPath(string const & path)
{
	// checks for already absolute path
	string rTemp = replaceEnvironmentPath(path);
	if (FileName::isAbsolute(rTemp))
		return rTemp;

	string temp;
	string const copy = rTemp;

	// Split by next /
	rTemp = split(rTemp, temp, '/');

	if (temp == ".")
		return FileName::getcwd().absFileName() + '/' + rTemp;

	if (temp == "~")
		return Package::get_home_dir().absFileName() + '/' + rTemp;

	if (temp == "..")
		return makeAbsPath(copy).absFileName();

	// Don't know how to handle this
	return copy;
}


// Search the string for ${VAR} and $VAR and replace VAR using getenv.
string const replaceEnvironmentPath(string const & path)
{
	// ${VAR} is defined as
	// $\{[A-Za-z_][A-Za-z_0-9]*\}
	static string const envvar_br = "[$]\\{([A-Za-z_][A-Za-z_0-9]*)\\}";

	// $VAR is defined as:
	// $[A-Za-z_][A-Za-z_0-9]*
	static string const envvar = "[$]([A-Za-z_][A-Za-z_0-9]*)";

	static regex const envvar_br_re("(.*)" + envvar_br + "(.*)");
	static regex const envvar_re("(.*)" + envvar + "(.*)");
	string result = path;
	while (1) {
		smatch what;
		if (!regex_match(result, what, envvar_br_re)) {
			if (!regex_match(result, what, envvar_re))
				break;
		}
		string env_var = getEnv(what.str(2));
		result = what.str(1) + env_var + what.str(3);
	}
	return result;
}


// Return a command prefix for setting the environment of the TeX engine.
string latexEnvCmdPrefix(string const & path)
{
	if (path.empty() || lyxrc.texinputs_prefix.empty())
		return string();

	string const texinputs_prefix = os::latex_path_list(
			replaceCurdirPath(path, lyxrc.texinputs_prefix));
	string const sep = string(1, os::path_separator(os::TEXENGINE));
	string const texinputs = getEnv("TEXINPUTS");

	if (os::shell() == os::UNIX)
		return "env TEXINPUTS=\"." + sep + texinputs_prefix
					  + sep + texinputs + "\" ";
	else
		return "cmd /d /c set TEXINPUTS=." + sep + texinputs_prefix
						   + sep + texinputs + "&";
}


// Replace current directory in all elements of a path list with a given path.
string const replaceCurdirPath(string const & path, string const & pathlist)
{
	string const oldpathlist = replaceEnvironmentPath(pathlist);
	char const sep = os::path_separator();
	string newpathlist;

	for (size_t i = 0, k = 0; i != string::npos; k = i) {
		i = oldpathlist.find(sep, i);
		string p = oldpathlist.substr(k, i - k);
		if (FileName::isAbsolute(p)) {
			newpathlist += p;
		} else if (i > k) {
			size_t offset = 0;
			if (p == ".") {
				offset = 1;
			} else if (prefixIs(p, "./")) {
				offset = 2;
				while (p[offset] == '/')
					++offset;
			}
			newpathlist += addPath(path, p.substr(offset));
			if (suffixIs(p, "//"))
				newpathlist += '/';
		}
		if (i != string::npos) {
			newpathlist += sep;
			// Stop here if the last element is empty 
			if (++i == oldpathlist.length())
				break;
		}
	}
	return newpathlist;
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


string const unzippedFileName(string const & zipped_file)
{
	string const ext = getExtension(zipped_file);
	if (ext == "gz" || ext == "z" || ext == "Z")
		return changeExtension(zipped_file, string());
	return onlyPath(zipped_file) + "unzipped_" + onlyFileName(zipped_file);
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
	string const system = package().system_support().absFileName();
	if (prefixIs(str, system) && str != system)
		return from_utf8("[" + str.erase(0, system.length()) + "]");

	// replace /home/blah with ~/
	string const home = Package::get_home_dir().absFileName();
	if (!home.empty() && prefixIs(str, home))
		str = subst(str, home, "~");

	if (str.length() <= threshold)
		return from_utf8(os::external_path(str));

	string const prefix = ".../";
	docstring dstr = from_utf8(str);
	docstring temp;

	while (dstr.length() > threshold)
		dstr = split(dstr, temp, '/');

	// Did we shorten everything away?
	if (dstr.empty()) {
		// Yes, filename itself is too long.
		// Pick the start and the end of the filename.
		dstr = from_utf8(onlyFileName(path));
		docstring const head = dstr.substr(0, threshold / 2 - 3);

		docstring::size_type len = dstr.length();
		docstring const tail =
			dstr.substr(len - threshold / 2 - 2, len - 1);
		dstr = head + from_ascii("...") + tail;
	}

	return from_utf8(os::external_path(prefix + to_utf8(dstr)));
}


#ifdef HAVE_READLINK
bool readLink(FileName const & file, FileName & link)
{
	string const encoded = file.toFilesystemEncoding();
#ifdef HAVE_DEF_PATH_MAX
	char linkbuffer[PATH_MAX + 1];
	int const nRead = ::readlink(encoded.c_str(),
				     linkbuffer, sizeof(linkbuffer) - 1);
	if (nRead <= 0)
		return false;
	linkbuffer[nRead] = '\0'; // terminator
#else
	vector<char> buf(1024);
	int nRead = -1;

	while (true) {
		nRead = ::readlink(encoded.c_str(), &buf[0], buf.size() - 1);
		if (nRead < 0) {
			return false;
		}
		if (nRead < buf.size() - 1) {
			break;
		}
		buf.resize(buf.size() * 2);
	}
	buf[nRead] = '\0'; // terminator
	const char * linkbuffer = &buf[0];
#endif
	link = makeAbsPath(linkbuffer, onlyPath(file.absFileName()));
	return true;
}
#else
bool readLink(FileName const &, FileName &)
{
	return false;
}
#endif


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

#if defined (_WIN32)
	int fno;
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	SECURITY_ATTRIBUTES security;
	HANDLE in, out;
	FILE * inf = 0;
	bool err2out = false;
	string command;
	string const infile = trim(split(cmd, command, '<'), " \"");
	command = rtrim(command);
	if (suffixIs(command, "2>&1")) {
		command = rtrim(command, "2>&1");
		err2out = true;
	}
	string const cmdarg = "/d /c " + command;
	string const comspec = getEnv("COMSPEC");

	security.nLength = sizeof(SECURITY_ATTRIBUTES);
	security.bInheritHandle = TRUE;
	security.lpSecurityDescriptor = NULL;

	if (CreatePipe(&in, &out, &security, 0)) {
		memset(&startup, 0, sizeof(STARTUPINFO));
		memset(&process, 0, sizeof(PROCESS_INFORMATION));

		startup.cb = sizeof(STARTUPINFO);
		startup.dwFlags = STARTF_USESTDHANDLES;

		startup.hStdError = err2out ? out : GetStdHandle(STD_ERROR_HANDLE);
		startup.hStdInput = infile.empty()
			? GetStdHandle(STD_INPUT_HANDLE)
			: CreateFile(infile.c_str(), GENERIC_READ,
				FILE_SHARE_READ, &security, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);
		startup.hStdOutput = out;

		if (startup.hStdInput != INVALID_HANDLE_VALUE &&
			CreateProcess(comspec.c_str(), (LPTSTR)cmdarg.c_str(),
				&security, &security, TRUE, CREATE_NO_WINDOW,
				0, 0, &startup, &process)) {

			CloseHandle(process.hThread);
			fno = _open_osfhandle((long)in, _O_RDONLY);
			CloseHandle(out);
			inf = _fdopen(fno, "r");
		}
	}
#elif defined (HAVE_POPEN)
	FILE * inf = ::popen(cmd.c_str(), os::popen_read_mode());
#elif defined (HAVE__POPEN)
	FILE * inf = ::_popen(cmd.c_str(), os::popen_read_mode());
#else
#error No popen() function.
#endif

	// (Claus Hentschel) Check if popen was successful ;-)
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

#if defined (_WIN32)
	WaitForSingleObject(process.hProcess, INFINITE);
	if (!infile.empty())
		CloseHandle(startup.hStdInput);
	CloseHandle(process.hProcess);
	int const pret = fclose(inf);
#elif defined (HAVE_PCLOSE)
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
	if (absfile.exists())
		return absfile;

	// Now we try to find it using kpsewhich.
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

	LYXERR(Debug::LATEX, "kpse status = " << c.first << '\n'
		 << "kpse result = `" << rtrim(c.second, "\n\r") << '\'');
	if (c.first != -1)
		return FileName(rtrim(to_utf8(from_filesystem8bit(c.second)), "\n\r"));
	else
		return FileName();
}


int compare_timestamps(FileName const & file1, FileName const & file2)
{
	// If the original is newer than the copy, then copy the original
	// to the new directory.

	int cmp = 0;
	if (file1.exists() && file2.exists()) {
		double const tmp = difftime(file1.lastModified(), file2.lastModified());
		if (tmp != 0)
			cmp = tmp > 0 ? 1 : -1;

	} else if (file1.exists()) {
		cmp = 1;
	} else if (file2.exists()) {
		cmp = -1;
	}

	return cmp;
}


bool prefs2prefs(FileName const & filename, FileName const & tempfile, bool lfuns)
{
	FileName const script = libFileSearch("scripts", "prefs2prefs.py");
	if (script.empty()) {
		LYXERR0("Could not find bind file conversion "
				"script prefs2prefs.py.");
		return false;
	}

	ostringstream command;
	command << os::python() << ' ' << quoteName(script.toFilesystemEncoding())
	  << ' ' << (lfuns ? "-l" : "-p") << ' '
		<< quoteName(filename.toFilesystemEncoding())
		<< ' ' << quoteName(tempfile.toFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::FILES, "Running `" << command_str << '\'');

	cmd_ret const ret = runCommand(command_str);
	if (ret.first != 0) {
		LYXERR0("Could not run file conversion script prefs2prefs.py.");
		return false;
	}
	return true;
}

int fileLock(const char * lock_file)
{
	int fd = -1;
#if defined(HAVE_LOCKF)
	fd = open(lock_file, O_CREAT|O_APPEND|O_SYNC|O_RDWR, 0666);
	if (lockf(fd, F_LOCK, 0) != 0) {
		close(fd);
		return(-1);
	}
#endif
	return(fd);
}

void fileUnlock(int fd, const char * /* lock_file*/)
{
#if defined(HAVE_LOCKF)
	if (fd >= 0) {
		if (lockf(fd, F_ULOCK, 0))
			LYXERR0("Can't unlock the file.");
		close(fd);
	}
#endif
}

} //namespace support
} // namespace lyx
