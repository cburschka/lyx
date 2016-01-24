// -*- C++ -*-
/**
 * \file package.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/Package.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"

#if defined (USE_WINDOWS_PACKAGING)
# include "support/os_win32.h"
#endif


#include <iostream>
#include <list>

#if !defined (USE_WINDOWS_PACKAGING) && \
    !defined (USE_MACOSX_PACKAGING) && \
    !defined (USE_HAIKU_PACKAGING) && \
    !defined (USE_POSIX_PACKAGING)
#error USE_FOO_PACKAGING must be defined for FOO = WINDOWS, MACOSX, HAIKU or POSIX.
#endif

#if defined (USE_MACOSX_PACKAGING)
# include "support/qstring_helpers.h"
# include <QDir>
# include <QDesktopServices>
#endif

using namespace std;

namespace lyx {
namespace support {

namespace {

Package package_;
bool initialised_ = false;

} // namespace anon


void init_package(string const & command_line_arg0,
		  string const & command_line_system_support_dir,
		  string const & command_line_user_support_dir)
{
	package_ = Package(command_line_arg0,
			   command_line_system_support_dir,
			   command_line_user_support_dir);
	initialised_ = true;
}


Package const & package()
{
	LAPPERR(initialised_);
	return package_;
}


namespace {

FileName const abs_path_from_binary_name(string const & exe);


bool inBuildDir(FileName const & abs_binary, FileName &, FileName &);

FileName findLyxBinary(FileName const & abs_binary);

FileName const get_document_dir(FileName const & home_dir);

FileName const get_locale_dir(FileName const & system_support_dir);

FileName const get_system_support_dir(FileName const & abs_binary,
				    string const & command_line_system_support_dir);

FileName const get_default_user_support_dir(FileName const & home_dir);

bool userSupportDir(FileName const & default_user_support_dir,
		     string const & command_line_user_support_dir, FileName & result);

string const & with_version_suffix();

string const fix_dir_name(string const & name);

} // namespace anon


Package::Package(string const & command_line_arg0,
		 string const & command_line_system_support_dir,
		 string const & command_line_user_support_dir)
	: explicit_user_support_dir_(false)
{
	// Specification of temp_dir_ may be reset by LyXRC,
	// but the default is fixed for a given OS.
	system_temp_dir_ = FileName::tempPath();
	temp_dir_ = system_temp_dir_;
	document_dir_ = get_document_dir(get_home_dir());

	FileName const abs_binary = abs_path_from_binary_name(command_line_arg0);
	binary_dir_ = FileName(onlyPath(abs_binary.absFileName()));

	// the LyX package directory
	lyx_dir_ = FileName(addPath(binary_dir_.absFileName(), "../"));
	lyx_dir_ = FileName(lyx_dir_.realPath());

	// Is LyX being run in-place from the build tree?
	in_build_dir_ = inBuildDir(abs_binary, build_support_dir_, system_support_dir_);

	if (!in_build_dir_) {
		system_support_dir_ =
			get_system_support_dir(abs_binary,
					       command_line_system_support_dir);
	}

	// Find the LyX executable
	lyx_binary_ = findLyxBinary(abs_binary);

	locale_dir_ = get_locale_dir(system_support_dir_);

	FileName const default_user_support_dir =
		get_default_user_support_dir(get_home_dir());

	explicit_user_support_dir_ = userSupportDir(default_user_support_dir,
				     command_line_user_support_dir, user_support_dir_);


	LYXERR(Debug::INIT, "<package>\n"
		<< "\tbinary_dir " << binary_dir().absFileName() << '\n'
		<< "\tsystem_support " << system_support().absFileName() << '\n'
		<< "\tbuild_support " << build_support().absFileName() << '\n'
		<< "\tuser_support " << user_support().absFileName() << '\n'
		<< "\tlocale_dir " << locale_dir().absFileName() << '\n'
		<< "\tdocument_dir " << document_dir().absFileName() << '\n'
		<< "\ttemp_dir " << temp_dir().absFileName() << '\n'
		<< "\thome_dir " << get_home_dir().absFileName() << '\n'
		<< "</package>\n");
}


int Package::reconfigureUserLyXDir(string const & option) const
{
	if (configure_command_.empty()) {
		FileName const configure_script(addName(system_support().absFileName(), "configure.py"));
		configure_command_ = os::python() + ' ' +
			quoteName(configure_script.toFilesystemEncoding()) +
			with_version_suffix() + " --binary-dir=" +
			quoteName(FileName(binary_dir().absFileName()).toFilesystemEncoding());
	}

	lyxerr << to_utf8(_("LyX: reconfiguring user directory")) << endl;
	PathChanger p(user_support());
	Systemcall one;
	int const ret = one.startscript(Systemcall::Wait, configure_command_ + option);
	lyxerr << "LyX: " << to_utf8(_("Done!")) << endl;
	return ret;
}


string Package::getConfigureLockName() const
{
	return addName(user_support().absFileName(), ".lyx_configure_lock");
}


void Package::set_temp_dir(FileName const & temp_dir) const
{
	if (temp_dir.empty())
		temp_dir_ = system_temp_dir_;
	else
		temp_dir_ = temp_dir;
}


FileName Package::messages_file(string const & c) const
{
	if (in_build_dir_) {
		FileName res = FileName(lyx_dir().absFileName() + "/../po/" + c + ".gmo");
		if (!res.isReadableFile())
			res = FileName(top_srcdir().absFileName() + "/po/" + c + ".gmo");
		return res;
	} else
		return FileName(locale_dir_.absFileName() + "/" + c
			+ "/LC_MESSAGES/" PACKAGE ".mo");
}


// The specification of home_dir_ is fixed for a given OS.
// A typical example on Windows: "C:/Documents and Settings/USERNAME"
// and on a Posix-like machine: "/home/USERNAME".
FileName const & Package::get_home_dir()
{
#if defined (USE_WINDOWS_PACKAGING)
	static FileName const home_dir(getEnv("USERPROFILE"));
#elif defined (USE_MACOSX_PACKAGING)
	static FileName const home_dir(fromqstr(QDir::homePath()));
#else // Posix-like.
	static FileName const home_dir(getEnv("HOME"));
#endif
	return home_dir;
}


namespace {

// These next functions contain the stuff that is substituted at
// configuration-time.
FileName const hardcoded_localedir()
{
	// FIXME UNICODE
	// The build system needs to make sure that this is in utf8 encoding.
	return FileName(LYX_ABS_INSTALLED_LOCALEDIR);
}


FileName const hardcoded_system_support_dir()
{
	// FIXME UNICODE
	// The build system needs to make sure that this is in utf8 encoding.
	return FileName(LYX_ABS_INSTALLED_DATADIR);
}


string const & with_version_suffix()
{
	static string const program_suffix = PROGRAM_SUFFIX;
	static string const with_version_suffix =
		" --with-version-suffix=" PROGRAM_SUFFIX;
	return program_suffix.empty() ? program_suffix : with_version_suffix;
}

} // namespace anon


FileName const & Package::top_srcdir()
{
	// FIXME UNICODE
	// The build system needs to make sure that this is in utf8 encoding.
	static FileName const dir(LYX_ABS_TOP_SRCDIR);
	return dir;
}


namespace {

bool check_command_line_dir(string const & dir,
			    string const & file,
			    string const & command_line_switch);

FileName const extract_env_var_dir(string const & env_var);

bool check_env_var_dir(FileName const & dir,
		       string const & env_var);

bool check_env_var_dir(FileName const & dir,
		       string const & file,
		       string const & env_var);

string const relative_locale_dir();

string const relative_system_support_dir();


/**
 * Convert \p name to internal path and strip a trailing slash, since it
 * comes from user input (commandline or environment).
 * \p name is encoded in utf8.
 */
string const fix_dir_name(string const & name)
{
	return rtrim(os::internal_path(name), "/");
}



bool isBuildDir(FileName const & abs_binary, string const & dir_location,
	FileName & build_support_dir)
{
    string search_dir = onlyPath(abs_binary.absFileName()) + dir_location;

    // Makefile by automake
    build_support_dir = FileName(addPath(search_dir, "lib"));
    if (!fileSearch(build_support_dir.absFileName(), "Makefile").empty()) {
        return true;
    }
    //  cmake file, no Makefile in lib
    FileName build_boost_dir = FileName(addPath(search_dir + "/3rdparty", "boost"));
    if (!fileSearch(build_boost_dir.absFileName(), "cmake_install.cmake").empty()) {
        return true;
    }

    return false;
}

bool inBuildDir(FileName const & abs_binary,
	FileName & build_support_dir, FileName & system_support_dir)
{
	string const check_text = "Checking whether LyX is run in place...";

	// We're looking for "Makefile" in a directory
	//   binary_dir/../lib
	// We're also looking for "chkconfig.ltx" in a directory
	//   top_srcdir()/lib
	// If both are found, then we're running LyX in-place.

	// Note that the name of the lyx binary may be a symbolic link.
	// If that is the case, then we follow the links too.
    FileName binary = abs_binary;
	while (true) {
		// Try and find "lyxrc.defaults".
		if( isBuildDir(binary, "../", build_support_dir) ||
            isBuildDir(binary, "../../", build_support_dir))
        {
			// Try and find "chkconfig.ltx".
			system_support_dir =
				FileName(addPath(Package::top_srcdir().absFileName(), "lib"));

			if (!fileSearch(system_support_dir.absFileName(), "chkconfig.ltx").empty()) {
				LYXERR(Debug::INIT, check_text << " yes");
				return true;
			}
		}

		// Check whether binary is a symbolic link.
		// If so, resolve it and repeat the exercise.
		if (!binary.isSymLink())
			break;

		FileName link;
		if (readLink(binary, link)) {
			binary = link;
		} else {
			// Unable to resolve the link.
			break;
		}
	}

	LYXERR(Debug::INIT, check_text << " no");
	system_support_dir = FileName();
	build_support_dir = FileName();

    return false;
}


bool doesFileExist(FileName & result, string const & search_dir, string const & name)
{
    result = fileSearch(search_dir, name);
    if (!result.empty()) {
        return true;
    }
    return false;
}


bool lyxBinaryPath(FileName & lyx_binary, string const & search_dir, string const & ext)
{
    lyx_binary = FileName();
    if(false) {   
    } else if (doesFileExist(lyx_binary, search_dir, "lyx" + ext)) {
    } else if (doesFileExist(lyx_binary, search_dir, "LyX" + ext)) {
    } else if (doesFileExist(lyx_binary, search_dir, "lyx" + string(PROGRAM_SUFFIX) + ext)) {
    } else if (doesFileExist(lyx_binary, search_dir, "LyX" + string(PROGRAM_SUFFIX) + ext)){
    }
    return !lyx_binary.empty() ? true : false;
}


FileName findLyxBinary(FileName const & abs_binary)
{
    string ext;
    string checkname = abs_binary.toFilesystemEncoding();
    int check_len = checkname.length();
    int prgsuffixlen = string(PROGRAM_SUFFIX).length();
    if ((prgsuffixlen > 0) && (check_len > prgsuffixlen) &&
       (checkname.substr(check_len-prgsuffixlen) == string(PROGRAM_SUFFIX))) {
	ext = "";
    }
    else if (!abs_binary.extension().empty()) {
        ext = "." + abs_binary.extension();
    }
    
    string binary_dir = onlyPath(abs_binary.absFileName());
      
    FileName lyx_binary;
    if (lyxBinaryPath(lyx_binary, binary_dir, ext))
        return lyx_binary;

    string search_dir = onlyPath(FileName(addPath(binary_dir, "/../")).absFileName());
    if (lyxBinaryPath(lyx_binary, search_dir, ext))
        return lyx_binary;
    
    return FileName();
}


// Specification of document_dir_ may be reset by LyXRC,
// but the default is fixed for a given OS.
FileName const get_document_dir(FileName const & home_dir)
{
#if defined (USE_WINDOWS_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.
	os::GetFolderPath win32_folder_path;
	return FileName(win32_folder_path(os::GetFolderPath::PERSONAL));
#elif defined (USE_MACOSX_PACKAGING) && (QT_VERSION >= 0x050000)
	(void)home_dir; // Silence warning about unused variable.
	return FileName(fromqstr(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
#elif defined (USE_MACOSX_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.
	return FileName(fromqstr(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
#else // Posix-like.
	return home_dir;
#endif
}



// Several sources are probed to ascertain the locale directory.
// The only requirement is that the result is indeed a directory.
FileName const get_locale_dir(FileName const & system_support_dir)
{
	// 1. Use the "LYX_LOCALEDIR" environment variable.
	FileName const path_env = extract_env_var_dir("LYX_LOCALEDIR");
	if (!path_env.empty() && check_env_var_dir(path_env, "LYX_LOCALEDIR"))
		return path_env;

	// 2. Search for system_support_dir / <relative locale dir>
	// The <relative locale dir> is OS-dependent. (On Unix, it will
	// be "../locale/".)
	FileName path(addPath(system_support_dir.absFileName(),
		relative_locale_dir()));

	if (path.exists() && path.isDirectory())
		return path;

	// 3. Fall back to the hard-coded LOCALEDIR.
	path = hardcoded_localedir();
	if (path.exists() && path.isDirectory())
		return path;

	return FileName();
}


// Extracts the absolute path from the foo of "-sysdir foo" or "-userdir foo"
FileName const abs_path_from_command_line(string const & command_line)
{
	if (command_line.empty())
		return FileName();

	string const str_path = fix_dir_name(command_line);
	return makeAbsPath(str_path);
}


// Does the grunt work for abs_path_from_binary_name()
FileName const get_binary_path(string const & exe)
{
#if defined (USE_WINDOWS_PACKAGING)
	// The executable may have been invoked either with or
	// without the .exe extension.
	// Ensure that it is present.
	string const as_internal_path = os::internal_path(exe);
	string const exe_path = suffixIs(as_internal_path, ".exe") ?
		as_internal_path : as_internal_path + ".exe";
#else
	string const exe_path = os::internal_path(exe);
#endif
	if (FileName::isAbsolute(exe_path))
		return FileName(exe_path);

	// Two possibilities present themselves.
	// 1. The binary is relative to the CWD.
	FileName const abs_exe_path = makeAbsPath(exe_path);
	if (abs_exe_path.exists())
		return abs_exe_path;

	// 2. exe must be the name of the binary only and it
	// can be found on the PATH.
	string const exe_name = onlyFileName(exe_path);
	if (exe_name != exe_path)
		return FileName();

	vector<string> const path = getEnvPath("PATH");
	vector<string>::const_iterator it = path.begin();
	vector<string>::const_iterator const end = path.end();
	for (; it != end; ++it) {
		// This will do nothing if *it is already absolute.
		string const exe_dir = makeAbsPath(*it).absFileName();

		FileName const exe_path(addName(exe_dir, exe_name));
		if (exe_path.exists())
			return exe_path;
	}

	// Didn't find anything.
	return FileName();
}


// Extracts the absolute path to the binary name received as argv[0].
FileName const abs_path_from_binary_name(string const & exe)
{
	FileName const abs_binary = get_binary_path(exe);
	if (abs_binary.empty()) {
		// FIXME UNICODE
		throw ExceptionMessage(ErrorException,
			_("LyX binary not found"),
			bformat(_("Unable to determine the path to the LyX binary from the command line %1$s"),
				from_utf8(exe)));
	}
	return abs_binary;
}


// A plethora of directories is searched to ascertain the system
// lyxdir which is defined as the first directory to contain
// "chkconfig.ltx".
FileName const
get_system_support_dir(FileName const & abs_binary,
		  string const & command_line_system_support_dir)
{
	string const chkconfig_ltx = "chkconfig.ltx";

	// searched_dirs is used for diagnostic purposes only in the case
	// that "chkconfig.ltx" is not found.
	list<FileName> searched_dirs;

	// 1. Use the -sysdir command line parameter.
	FileName path = abs_path_from_command_line(command_line_system_support_dir);
	if (!path.empty()) {
		searched_dirs.push_back(path);
		if (check_command_line_dir(path.absFileName(), chkconfig_ltx, "-sysdir"))
			return path;
	}

	// 2. Use the "LYX_DIR_${major}${minor}x" environment variable.
	path = extract_env_var_dir(LYX_DIR_VER);
	if (!path.empty()) {
		searched_dirs.push_back(path);
		if (check_env_var_dir(path, chkconfig_ltx, LYX_DIR_VER))
			return path;
	}

	// 3. Search relative to the lyx binary.
	// We're looking for "chkconfig.ltx" in a directory
	//   OnlyPath(abs_binary) / <relative dir> / PACKAGE /
	// PACKAGE is hardcoded in config.h. Eg "lyx" or "lyx-1.3.6cvs".
	// <relative dir> is OS-dependent; on Unix, it will be "../share/".
	string const relative_lyxdir = relative_system_support_dir();

	// One subtlety to be aware of. The name of the lyx binary may be
	// a symbolic link. If that is the case, then we follow the links too.
	FileName binary = abs_binary;
	while (true) {
		// Try and find "chkconfig.ltx".
		string const binary_dir = onlyPath(binary.absFileName());

		FileName const lyxdir(addPath(binary_dir, relative_lyxdir));
		searched_dirs.push_back(lyxdir);

		if (!fileSearch(lyxdir.absFileName(), chkconfig_ltx).empty()) {
			// Success! "chkconfig.ltx" has been found.
			return lyxdir;
		}

		// Check whether binary is a symbolic link.
		// If so, resolve it and repeat the exercise.
		if (!binary.isSymLink())
			break;

		FileName link;
		if (readLink(binary, link)) {
			binary = link;
		} else {
			// Unable to resolve the link.
			break;
		}
	}

	// 4. Repeat the exercise on the directory itself.
	FileName binary_dir(onlyPath(abs_binary.absFileName()));
	while (true) {
		// This time test whether the directory is a symbolic link
		// *before* looking for "chkconfig.ltx".
		// (We've looked relative to the original already.)
		if (!binary.isSymLink())
			break;

		FileName link;
		if (readLink(binary_dir, link)) {
			binary_dir = link;
		} else {
			// Unable to resolve the link.
			break;
		}

		// Try and find "chkconfig.ltx".
		FileName const lyxdir(addPath(binary_dir.absFileName(),
			relative_lyxdir));
		searched_dirs.push_back(lyxdir);

		if (!fileSearch(lyxdir.absFileName(), chkconfig_ltx).empty()) {
			// Success! "chkconfig.ltx" has been found.
			return lyxdir;
		}
	}

	// 5. In desparation, try the hard-coded system support dir.
	path = hardcoded_system_support_dir();
	if (!fileSearch(path.absFileName(), chkconfig_ltx).empty())
		return path;

	// Everything has failed :-(
	// So inform the user and exit.
	string searched_dirs_str;
	typedef list<FileName>::const_iterator iterator;
	iterator const begin = searched_dirs.begin();
	iterator const end = searched_dirs.end();
	for (iterator it = begin; it != end; ++it) {
		if (it != begin)
			searched_dirs_str += "\n\t";
		searched_dirs_str += it->absFileName();
	}

	// FIXME UNICODE
	throw ExceptionMessage(ErrorException, _("No system directory"),
		bformat(_("Unable to determine the system directory "
				"having searched\n"
				"\t%1$s\n"
				"Use the '-sysdir' command line parameter or "
				"set the environment variable\n%2$s "
				"to the LyX system directory containing the "
				"file `chkconfig.ltx'."),
			  from_utf8(searched_dirs_str), from_ascii(LYX_DIR_VER)));

	// Keep the compiler happy.
	return FileName();
}


// Returns the absolute path to the user lyxdir, together with a flag
// indicating whether this directory was specified explicitly (as -userdir
// or through an environment variable) or whether it was deduced.
bool userSupportDir(FileName const & default_user_support_dir,
	string const & command_line_user_support_dir, FileName & result)
{
	// 1. Use the -userdir command line parameter.
	result = abs_path_from_command_line(command_line_user_support_dir);
	if (!result.empty())
		return true;

	// 2. Use the LYX_USERDIR_${major}${minor}x environment variable.
	result = extract_env_var_dir(LYX_USERDIR_VER);
	if (!result.empty())
		return true;

	// 3. Use the OS-dependent default_user_support_dir
	result = default_user_support_dir;
	return false;
}


// $HOME/.lyx on POSIX but on Win32 it will be something like
// "C:/Documents and Settings/USERNAME/Application Data/LyX"
FileName const get_default_user_support_dir(FileName const & home_dir)
{
#if defined (USE_WINDOWS_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.

	os::GetFolderPath win32_folder_path;
	return FileName(addPath(win32_folder_path(os::GetFolderPath::APPDATA), PACKAGE));

#elif defined (USE_MACOSX_PACKAGING) && (QT_VERSION >= 0x050000)
	(void)home_dir; // Silence warning about unused variable.
	return FileName(addPath(fromqstr(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)), PACKAGE));

#elif defined (USE_MACOSX_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.
	return FileName(addPath(fromqstr(QDesktopServices::storageLocation(QDesktopServices::DataLocation)), PACKAGE));

#elif defined (USE_HAIKU_PACKAGING)
	return FileName(addPath(home_dir.absFileName(), string("/config/settings/") + PACKAGE));

#else // USE_POSIX_PACKAGING
	return FileName(addPath(home_dir.absFileName(), string(".") + PACKAGE));
#endif
}


// Check that directory @c dir contains @c file.
// Else emit an error message about an invalid @c command_line_switch.
bool check_command_line_dir(string const & dir,
			    string const & file,
			    string const & command_line_switch)
{
	FileName const abs_path = fileSearch(dir, file);
	if (abs_path.empty()) {
		// FIXME UNICODE
		throw ExceptionMessage(ErrorException, _("File not found"), bformat(
			_("Invalid %1$s switch.\nDirectory %2$s does not contain %3$s."),
			from_utf8(command_line_switch), from_utf8(dir),
			from_utf8(file)));
	}

	return !abs_path.empty();
}


// The environment variable @c env_var expands to a (single) file path.
FileName const extract_env_var_dir(string const & env_var)
{
	string const dir = fix_dir_name(getEnv(env_var));
	return dir.empty() ? FileName() : makeAbsPath(dir);
}


// Check that directory @c dir contains @c file.
// Else emit a warning about an invalid @c env_var.
bool check_env_var_dir(FileName const & dir,
		       string const & file,
		       string const & env_var)
{
	FileName const abs_path = fileSearch(dir.absFileName(), file);
	if (abs_path.empty()) {
		// FIXME UNICODE
		throw ExceptionMessage(WarningException, _("File not found"), bformat(
			_("Invalid %1$s environment variable.\n"
				"Directory %2$s does not contain %3$s."),
			from_utf8(env_var), from_utf8(dir.absFileName()),
			from_utf8(file)));
	}

	return !abs_path.empty();
}


// Check that directory @c dir is indeed a directory.
// Else emit a warning about an invalid @c env_var.
bool check_env_var_dir(FileName const & dir,
		       string const & env_var)
{
	bool const success = dir.exists() && dir.isDirectory();

	if (!success) {
		// Put this string on a single line so that the gettext
		// search mechanism in po/Makefile.in.in will register
		// Package.cpp.in as a file containing strings that need
		// translation.
		// FIXME UNICODE
		docstring const fmt =
			_("Invalid %1$s environment variable.\n%2$s is not a directory.");

		throw ExceptionMessage(WarningException, _("Directory not found"), bformat(
			fmt, from_utf8(env_var), from_utf8(dir.absFileName())));
	}

	return success;
}


// The locale directory relative to the LyX system directory.
string const relative_locale_dir()
{
#if defined (USE_WINDOWS_PACKAGING) || defined (USE_MACOSX_PACKAGING)
	return "locale/";
#else
	return "../locale/";
#endif
}


// The system lyxdir is relative to the directory containing the LyX binary.
string const relative_system_support_dir()
{
	string result;

#if defined (USE_WINDOWS_PACKAGING) || defined (USE_MACOSX_PACKAGING)
	result = "../Resources/";
#else // Posix-like.
	result = addPath("../share/", PACKAGE);
#endif

	return result;
}

} // namespace anon

} // namespace support
} // namespace lyx
