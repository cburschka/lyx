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
#include "support/lstrings.h"
#include "support/os.h"

#if defined (USE_WINDOWS_PACKAGING)
# include "support/os_win32.h"
#endif


#include <list>

#if !defined (USE_WINDOWS_PACKAGING) && \
    !defined (USE_MACOSX_PACKAGING) && \
    !defined (USE_POSIX_PACKAGING)
#error USE_FOO_PACKAGING must be defined for FOO = WINDOWS, MACOSX or POSIX.
#endif

#if defined (USE_MACOSX_PACKAGING)
# include <CoreServices/CoreServices.h> // FSFindFolder, FSRefMakePath
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
		  string const & command_line_user_support_dir,
		  exe_build_dir_to_top_build_dir top_build_dir_location)
{
	// Can do so only once.
	if (initialised_)
		return;

	package_ = Package(command_line_arg0,
			   command_line_system_support_dir,
			   command_line_user_support_dir,
			   top_build_dir_location);
	initialised_ = true;
}


Package const & package()
{
	// Commented out because package().locale_dir() can be called
	// from the message translation code in Messages.cpp before
	// init_package() is called. Lars is on the case...
	// LASSERT(initialised_, /**/);
	return package_;
}


namespace {

FileName const abs_path_from_binary_name(string const & exe);

void buildDirs(FileName const & abs_binary,
	exe_build_dir_to_top_build_dir top_build_dir_location,
	FileName &, FileName &);

FileName const get_document_dir(FileName const & home_dir);

FileName const get_home_dir();

FileName const get_locale_dir(FileName const & system_support_dir);

FileName const get_system_support_dir(FileName const & abs_binary,
				    string const & command_line_system_support_dir);

FileName const get_default_user_support_dir(FileName const & home_dir);

bool userSupportDir(FileName const & default_user_support_dir,
		     string const & command_line_user_support_dir, FileName & result);


string const & with_version_suffix();

} // namespace anon


Package::Package(string const & command_line_arg0,
		 string const & command_line_system_support_dir,
		 string const & command_line_user_support_dir,
		 exe_build_dir_to_top_build_dir top_build_dir_location)
	: explicit_user_support_dir_(false)
{
	home_dir_ = get_home_dir();
	// Specification of temp_dir_ may be reset by LyXRC,
	// but the default is fixed for a given OS.
	system_temp_dir_ = FileName::tempPath();
	temp_dir_ = system_temp_dir_;
	document_dir_ = get_document_dir(home_dir_);

	FileName const abs_binary = abs_path_from_binary_name(command_line_arg0);
	string const bdir = onlyPath(abs_binary.absFilename());
	// We may be using libtools
	if (suffixIs(bdir, ".libs/"))
		binary_dir_ = FileName(addPath(bdir, "../"));
	else
		binary_dir_ = FileName(bdir);

	// Is LyX being run in-place from the build tree?
	buildDirs(abs_binary, top_build_dir_location,
		build_support_dir_, system_support_dir_);

	if (build_support_dir_.empty())
		system_support_dir_ =
			get_system_support_dir(abs_binary,
					       command_line_system_support_dir);

	locale_dir_ = get_locale_dir(system_support_dir_);

	FileName const default_user_support_dir =
		get_default_user_support_dir(home_dir_);

	explicit_user_support_dir_ = userSupportDir(default_user_support_dir,
				     command_line_user_support_dir, user_support_dir_);

	FileName const configure_script(addName(system_support().absFilename(), "configure.py"));
	configure_command_ = os::python() + ' ' +
			quoteName(configure_script.toFilesystemEncoding()) +
			with_version_suffix();

	LYXERR(Debug::INIT, "<package>\n"
		<< "\tbinary_dir " << binary_dir().absFilename() << '\n'
		<< "\tsystem_support " << system_support().absFilename() << '\n'
		<< "\tbuild_support " << build_support().absFilename() << '\n'
		<< "\tuser_support " << user_support().absFilename() << '\n'
		<< "\tlocale_dir " << locale_dir().absFilename() << '\n'
		<< "\tdocument_dir " << document_dir().absFilename() << '\n'
		<< "\ttemp_dir " << temp_dir().absFilename() << '\n'
		<< "\thome_dir " << home_dir().absFilename() << '\n'
		<< "</package>\n");
}


void Package::set_temp_dir(FileName const & temp_dir) const
{
	if (temp_dir.empty())
		temp_dir_ = system_temp_dir_;
	else
		temp_dir_ = temp_dir;
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


FileName buildSupportDir(string const & binary_dir,
		      exe_build_dir_to_top_build_dir top_build_dir_location)
{
	string indirection;
	switch (top_build_dir_location) {
	case top_build_dir_is_one_level_up:
		indirection = "../lib";
		break;
	case top_build_dir_is_two_levels_up:
		indirection = "../../lib";
		break;
	}
	return FileName(addPath(binary_dir, indirection));
}


void buildDirs(FileName const & abs_binary,
  exe_build_dir_to_top_build_dir top_build_dir_location,
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
		string binary_dir = onlyPath(binary.absFilename());
		// We may be using libtools with static linking.
		if (suffixIs(binary_dir, ".libs/"))
			binary_dir = addPath(binary_dir, "../");
		build_support_dir = buildSupportDir(binary_dir, top_build_dir_location);
		if (!fileSearch(build_support_dir.absFilename(), "Makefile").empty()) {
			// Try and find "chkconfig.ltx".
			system_support_dir =
				FileName(addPath(Package::top_srcdir().absFilename(), "lib"));

			if (!fileSearch(system_support_dir.absFilename(), "chkconfig.ltx").empty()) {
				LYXERR(Debug::INIT, check_text << " yes");
				return;
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
}


// Specification of document_dir_ may be reset by LyXRC,
// but the default is fixed for a given OS.
FileName const get_document_dir(FileName const & home_dir)
{
#if defined (USE_WINDOWS_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.
	os::GetFolderPath win32_folder_path;
	return FileName(win32_folder_path(os::GetFolderPath::PERSONAL));
#else // Posix-like.
	return home_dir;
#endif
}


// The specification of home_dir_ is fixed for a given OS.
// A typical example on Windows: "C:/Documents and Settings/USERNAME"
// and on a Posix-like machine: "/home/USERNAME".
FileName const get_home_dir()
{
#if defined (USE_WINDOWS_PACKAGING)
	string const home_dir = getEnv("USERPROFILE");
#else // Posix-like.
	string const home_dir = getEnv("HOME");
#endif

	return FileName(fix_dir_name(home_dir));
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
	FileName path(addPath(system_support_dir.absFilename(),
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
	FileName path(str_path);
	return path.isAbsolute() ? path : makeAbsPath(str_path);
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
	FileName exepath(exe_path);
	if (exepath.isAbsolute())
		return exepath;

	// Two possibilities present themselves.
	// 1. The binary is relative to the CWD.
	FileName const abs_exe_path = makeAbsPath(exe_path);
	if (abs_exe_path.exists())
		return abs_exe_path;

	// 2. exe must be the name of the binary only and it
	// can be found on the PATH.
	string const exe_name = onlyFilename(exe_path);
	if (exe_name != exe_path)
		return FileName();

	vector<string> const path = getEnvPath("PATH");
	vector<string>::const_iterator it = path.begin();
	vector<string>::const_iterator const end = path.end();
	for (; it != end; ++it) {
		// This will do nothing if *it is already absolute.
		string const exe_dir = makeAbsPath(*it).absFilename();

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
		if (check_command_line_dir(path.absFilename(), chkconfig_ltx, "-sysdir"))
			return path;
	}

	// 2. Use the "LYX_DIR_16x" environment variable.
	// FIXME We need to iherit the actual version number from elsewhere
	// otherwise we will forget to update this for new major releases
	path = extract_env_var_dir("LYX_DIR_16x");
	if (!path.empty()) {
		searched_dirs.push_back(path);
		if (check_env_var_dir(path, chkconfig_ltx, "LYX_DIR_16x"))
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
		string const binary_dir = onlyPath(binary.absFilename());

		FileName const lyxdir(addPath(binary_dir, relative_lyxdir));
		searched_dirs.push_back(lyxdir);

		if (!fileSearch(lyxdir.absFilename(), chkconfig_ltx).empty()) {
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
	FileName binary_dir(onlyPath(abs_binary.absFilename()));
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
		FileName const lyxdir(addPath(binary_dir.absFilename(),
			relative_lyxdir));
		searched_dirs.push_back(lyxdir);

		if (!fileSearch(lyxdir.absFilename(), chkconfig_ltx).empty()) {
			// Success! "chkconfig.ltx" has been found.
			return lyxdir;
		}
	}

	// 5. In desparation, try the hard-coded system support dir.
	path = hardcoded_system_support_dir();
	if (!fileSearch(path.absFilename(), chkconfig_ltx).empty())
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
		searched_dirs_str += it->absFilename();
	}

	// FIXME UNICODE
	throw ExceptionMessage(ErrorException, _("No system directory"),
		bformat(_("Unable to determine the system directory "
					 "having searched\n"
					 "\t%1$s\n"
					 "Use the '-sysdir' command line parameter or "
					 "set the environment variable LYX_DIR_16x to "
					 "the LyX system directory containing the file "
					 "`chkconfig.ltx'."),
			  from_utf8(searched_dirs_str)));

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

	// 2. Use the LYX_USERDIR_16x environment variable.
	result = extract_env_var_dir("LYX_USERDIR_16x");
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

#elif defined (USE_MACOSX_PACKAGING)
	(void)home_dir; // Silence warning about unused variable.

	FSRef fsref;
	OSErr const error_code =
		FSFindFolder(kUserDomain, kApplicationSupportFolderType,
			     kDontCreateFolder, &fsref);
	if (error_code != 0)
		return FileName();

	// FSRefMakePath returns the result in utf8
	char store[PATH_MAX + 1];
	OSStatus const status_code =
		FSRefMakePath(&fsref,
			      reinterpret_cast<UInt8*>(store), PATH_MAX);
	if (status_code != 0)
		return FileName();

	return FileName(addPath(reinterpret_cast<char const *>(store), PACKAGE));

#else // USE_POSIX_PACKAGING
	return FileName(addPath(home_dir.absFilename(), string(".") + PACKAGE));
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
	FileName const abs_path = fileSearch(dir.absFilename(), file);
	if (abs_path.empty()) {
		// FIXME UNICODE
		throw ExceptionMessage(WarningException, _("File not found"), bformat(
			_("Invalid %1$s environment variable.\n"
				"Directory %2$s does not contain %3$s."),
			from_utf8(env_var), from_utf8(dir.absFilename()),
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
			fmt, from_utf8(env_var), from_utf8(dir.absFilename())));
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
