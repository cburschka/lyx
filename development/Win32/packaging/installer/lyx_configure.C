// -*- C++ -*-
/*
 * \file lyx_configure.C
 * This file is part of LyX, the document processor.
 * http://www.lyx.org/
 * Licence details can be found in the file COPYING or copy at
 * http://www.lyx.org/about/license.php
 *
 * \author Angus Leeming
 * Full author contact details are available in file CREDITS or copy at
 * http://www.lyx.org/about/credits.php
 *
 * Define five functions that can be called from the NSIS installer:
 *
 * set_path_prefix [ configure_file, path_prefix ]
 * create_lyx_bat [ bin_dir, lang ]
 * create_relyx_bat [ bin_dir, lang ]
 * run_configure [ configure_file, path_prefix ]
 * set_env [ var_name, var_value ]
 *
 * The quantities in [ ... ] are the variables that the functions expect
 * to find on the stack. They push "-1" onto the stack on failure and "0"
 * onto the stack on success.
 *
 * Compile the code with
 *
 * g++ -I/c/Program\ Files/NSIS/Contrib -Wall -shared \
 *    lyx_configure.C -o lyx_configure.dll
 *
 * Compilation requires the exdll.h header file to be installed (here at
 * C:\Program Files\NSIS\Contrib\ExDLL\exdll.h). The file can be obtained
 * from http://cvs.sourceforge.net/viewcvs.py/nsis/NSIS/Contrib/ExDLL/
 *
 * Move the resulting lyx_configure.dll to C:\Program Files\NSIS\Plugins
 */

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <windows.h>
#include "ExDLL/exdll.h"


namespace {

std::string const subst(std::string const & a,
			std::string const & oldstr,
			std::string const & newstr)
{
	std::string lstr = a;
	std::string::size_type i = 0;
	std::string::size_type const olen = oldstr.length();
	while ((i = lstr.find(oldstr, i)) != std::string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length();
	}
	return lstr;
}


std::string const basename(std::string const & path)
{
	// Windows recognizes both '/' and '\' as directory separators.
	std::string::size_type const final_slash = path.find_last_of("/\\");
	return (final_slash == std::string::npos) ?
		path :
		path.substr(final_slash+1);
}


std::string const dirname(std::string const & path)
{
	// Windows recognizes both '/' and '\' as directory separators.
	std::string::size_type const final_slash = path.find_last_of("/\\");
	return (final_slash == std::string::npos) ?
		std::string() :
		path.substr(0, final_slash);
}


std::string const pop_from_stack()
{
	char data[10*MAX_PATH];
	return (popstring(data) == 0) ? data : std::string();
}


void push_to_stack(int const data)
{
	std::ostringstream os;
	os << data;
	pushstring(os.str().c_str());
}


std::list<std::string> const tokenize(std::string const & data,
				      char const separator)
{
	std::list<std::string> result;
	std::string::size_type index = 0;
	while (true) {
		std::string::size_type const end = data.find(separator, index);
		if (end == std::string::npos) {
			std::string::size_type const count =
				data.size() - index;
			std::string const elem = data.substr(index, count);
			if (elem.size() > 0) {
				result.push_back(elem);
			}
			break;
		}
		std::string::size_type const count = end - index;
		std::string const elem = data.substr(index, count);
		if (elem.size() > 0) {
			result.push_back(elem);
		}
		index = (end+1 == data.size()) ? std::string::npos : end + 1;
	}
	return result;
}


void remove_duplicates(std::list<std::string> & data)
{
	typedef std::list<std::string>::iterator iterator;
	for (iterator it = data.begin(); it != data.end(); ++it) {
		iterator next = it;
		++next;
		if (next == data.end())
			break;
		iterator end = std::remove(next, data.end(), *it);
		data.erase(end, data.end());
	}
}


std::string concatenate(std::list<std::string> const & data,
			char const separator)
{
	typedef std::list<std::string>::const_iterator iterator;
	iterator it = data.begin();
	iterator const end = data.end();
	if (it == end)
		return std::string();

	std::ostringstream result;
	result << *it;
	++it;
	for (; it != end; ++it) {
		result << separator << *it;
	}
	return result.str();
}


std::string const sanitize_win32_path(std::string const & in)
{
	std::string out = subst(in, "/", "\\");
	// Replace multiple adjacent directory separators with a single one
	typedef std::string::size_type size_type;
	size_type offset = 0;
	while (offset != std::string::npos) {
		size_type const slash = out.find_first_of('\\', offset);
		if (slash == std::string::npos)
			break;
		size_type const slash_end = out.find_first_not_of('\\', slash);

		if (slash_end == std::string::npos) {
			// Remove all trailing '\' characters
			out = out.substr(0, slash);
			break;

		} else {
			size_type const count = slash_end - (slash + 1);
			if (count > 0)
				out.erase(slash + 1, count);
		}
		offset = slash + 1;
	}

	return out;
}


std::string const sanitize_path_envvar(std::string const & in)
{
	// A Windows PATH environment variable has elements separated
	// by a ';' character.
	typedef std::list<std::string> string_list;
	string_list envvar = tokenize(in, ';');

	// Ensure that each PATH element uses '\' directory separators
	// and doesn't end in a '\'.
	string_list::iterator const end = envvar.end();
	for (string_list::iterator it = envvar.begin(); it != end; ++it) {
		std::string const new_path = sanitize_win32_path(*it);
		*it = new_path;
	}

	remove_duplicates(envvar);
	return concatenate(envvar, ';');
}


bool replace_path_prefix(std::string & data,
			 std::string::size_type const prefix_pos,
			 std::string const & path_prefix)
{
	std::string::size_type start_prefix =
		data.find_first_of('"', prefix_pos);
	if (start_prefix == std::string::npos)
		return false;
	start_prefix += 1;

	std::string::size_type end_line =
		data.find_first_of('\n', prefix_pos);
	if (end_line == std::string::npos)
		return false;

	std::string::size_type end_prefix =
		data.find_last_of('"', end_line);
	if (end_prefix == std::string::npos || end_prefix == start_prefix)
		return false;

	std::string::size_type const count = end_prefix - start_prefix;
	std::string const old_prefix = data.substr(start_prefix, count);
	// The configure script needs to see '\\' for each '\' directory
	// separator.
	std::string const prefix =
		subst(sanitize_path_envvar(path_prefix + ';' + old_prefix),
		      "\\", "\\\\");

	data.erase(start_prefix, count);
	data.insert(start_prefix, prefix);

	return true;
}


bool insert_path_prefix(std::string & data,
			std::string::size_type const xfonts_pos,
			std::string const & path_prefix)
{
	std::string::size_type const xfonts_start =
		data.find_last_of('\n', xfonts_pos);

	if (xfonts_start == std::string::npos)
		return false;

	// The configure script needs to see '\\' for each '\' directory
	// separator.
	std::string const prefix =
		subst(sanitize_path_envvar(path_prefix), "\\", "\\\\");

	std::ostringstream ss;
	ss << data.substr(0, xfonts_start)
	   << "\n"
	      "cat >>$outfile <<EOF\n"
	      "\n"
	      "\\\\path_prefix \"" << prefix << "\"\n"
	      "EOF\n"
	   << data.substr(xfonts_start);
	data = ss.str();

	return true;
}


bool write_bat(std::ostream & os, std::string const & quoted_exe)
{
	os << "if \"%~1\" == \"~1\" goto win95\n"
	   << quoted_exe << " %*\n"
	   << "goto end\n"
	   << ":win95\n"
	   << quoted_exe << " %1 %2 %3 %4 %5 %6 %7 %8 %9\n"
	   << ":end\n";

	return os;
}


// A wrapper for GetEnvironmentVariable
bool get_environment_variable(std::string const & var, std::string & val)
{
	if (var.empty())
		return false;

	std::vector<char> cval(MAX_PATH, '\0');
	DWORD size = GetEnvironmentVariable(var.c_str(),
					    &*cval.begin(),
					    cval.size());

	if (size == 0)
		// The specified variable was not found.
		return false;

	if (size > cval.size()) {
		// Insufficient size in the buffer to store the var
		// so resize to the required size.
		cval.resize(size, '\0');
		size = GetEnvironmentVariable(var.c_str(),
					      &*cval.begin(),
					      cval.size());
		if (size == 0 || size > cval.size())
			// Give up
			return false;
	}

	val.assign(cval.begin(), cval.begin() + size);
	return true;
}


// Inserts code into "configure" to output "path_prefix" to lyxrc.defaults.
// Throws std::runtime_error on failure.
void set_path_prefix(std::string const & configure_file,
		     std::string const & path_prefix)
{
	std::ifstream ifs(configure_file.c_str());
	if (!ifs) {
		std::ostringstream ss;
		ss << "Unable to open " << configure_file << " for reading";
		throw std::runtime_error(ss.str());
	}

	std::istreambuf_iterator<char> const begin_ifs(ifs);
	std::istreambuf_iterator<char> const end_ifs;
	std::string configure_data(begin_ifs, end_ifs);
	ifs.close();

	// Does configure already contain a "path_prefix" entry
	// or should we insert one?
	std::string::size_type const prefix_pos =
		configure_data.find("path_prefix");
	if (prefix_pos != std::string::npos) {
		if (!replace_path_prefix(configure_data, prefix_pos, path_prefix)) {
			std::ostringstream ss;
			ss << "Unable to replace the existing path_prefix in "
			   << configure_file;
			throw std::runtime_error(ss.str());
		}
	} else {
		std::string::size_type const xfonts_pos =
			configure_data.find("if [ \"x$use_cygwin_path_fix");

		if (xfonts_pos == std::string::npos) {
			std::ostringstream ss;
			ss << "Unable to find a position to insert the "
			      "path_prefix entry in "
			   << configure_file;
			throw std::runtime_error(ss.str());
		}

		if (!insert_path_prefix(configure_data, xfonts_pos, path_prefix)) {
			std::ostringstream ss;
			ss << "Unable to insert a new path_prefix in "
			   << configure_file;
			throw std::runtime_error(ss.str());
		}
	}

	std::ofstream ofs(configure_file.c_str());
	if (!ofs) {
		std::ostringstream ss;
		ss << "Unable to open " << configure_file << " for writing";
		throw std::runtime_error(ss.str());
	}

	ofs << configure_data;
	if (!ofs) {
		std::ostringstream ss;
		ss << "Failed to write " << configure_file;
		throw std::runtime_error(ss.str());
	}
}


// Creates the lyx.bat file in the LyX\bin folder.
// Throws std::runtime_error on failure.
void create_lyx_bat(std::string const & bin_dir, std::string const & lang)
{
	std::string const lyx_bat_file = bin_dir + "\\lyx.bat";
	std::ofstream ofs(lyx_bat_file.c_str());
	if (!ofs) {
		std::ostringstream ss;
		ss << "Unable to unable to open "
		   << lyx_bat_file << " for writing";
		throw std::runtime_error(ss.str());
	}

	ofs << "@echo off\n"
	    << "if \"%LANG%\"==\"\" SET LANG=" << lang << "\n";

	std::string const lyx_exe_file = bin_dir + "\\lyx.exe";
	if (!write_bat(ofs, "start \"LyX\" \"" + lyx_exe_file + "\"")) {
		std::ostringstream ss;
		ss << "Failed to write to " << lyx_bat_file;
		throw std::runtime_error(ss.str());
	}
}


// Creates the reLyX.bat file in the LyX\bin folder.
// Throws std::runtime_error on failure.
void create_relyx_bat(std::string const & bin_dir, std::string const & lang)
{
	std::string const relyx_bat_file = bin_dir + "\\reLyX.bat";
	std::ofstream ofs(relyx_bat_file.c_str());
	if (!ofs) {
		std::ostringstream ss;
		ss << "Unable to unable to open "
		   << relyx_bat_file << " for writing";
		throw std::runtime_error(ss.str());
	}

	std::string relyx_file = bin_dir + "\\reLyX";
	std::string relyx = "perl.exe \"" + relyx_file + "\"";
	if (!write_bat(ofs, "perl.exe \"" + relyx_file + "\"")) {
		std::ostringstream ss;
		ss << "Failed to write to " << relyx_bat_file;
		throw std::runtime_error(ss.str());
	}
}


// Runs "sh configure" to generate things like lyxrc.defaults.
// Throws std::runtime_error on failure.
void run_configure(std::string const & abs_configure_file_in,
		   std::string const & path_prefix)
{
	std::string const abs_configure_file =
		sanitize_win32_path(abs_configure_file_in);
	std::string const configure_dir = dirname(abs_configure_file);
	std::string const configure_file = basename(abs_configure_file);

	if (configure_file.empty() || configure_dir.empty()) {
		std::ostringstream ss;
		ss << "Invalid path to configure file: "
		   << abs_configure_file;
		throw std::runtime_error(ss.str());
	}

	if (SetCurrentDirectory(configure_dir.c_str()) == 0) {
		std::ostringstream ss;
		ss << "Unable to cd "<< configure_dir;
		throw std::runtime_error(ss.str());
	}

	std::string path;
	if (!get_environment_variable("PATH", path)) {
		std::ostringstream ss;
		ss << "Unable to grab the PATH environment variable";
		throw std::runtime_error(ss.str());
	}
	path = sanitize_path_envvar(path_prefix + ';' + path);
	if (SetEnvironmentVariable("PATH", path.c_str()) == 0) {
		std::ostringstream ss;
		ss << "Failed to set the PATH environment variable to:\n"
		   << path << '\n';
		throw std::runtime_error(ss.str());
	}

	// Even "start /WAIT /B sh.exe configure" returns
	// before the script is done, so just invoke "sh" directly.

	// Assumes that configure_file does not need quoting.
	std::string const command = std::string("sh.exe ") + configure_file;
	if (system(command.c_str()) != 0) {
		std::ostringstream ss;
		ss << "Failed to run: " << command;
		throw std::runtime_error(ss.str());
	}
}

} // namespace anon


//===========================================//
//                                           //
// Below is the public interface to the .dll //
//                                           //
//===========================================//

BOOL WINAPI DllMain(HANDLE /*hInst*/,
		    ULONG /*ul_reason_for_call*/,
		    LPVOID /*lpReserved*/)
{
	return TRUE;
}


// Inserts code into "configure" to output "path_prefix" to lyxrc.defaults.
extern "C"
void __declspec(dllexport) set_path_prefix(HWND const /*hwndParent*/,
					   int const string_size,
					   char * const variables,
					   stack_t ** const stacktop)
{
	EXDLL_INIT();

	std::string const configure_file = pop_from_stack();
	std::string const path_prefix = pop_from_stack();
	if (configure_file.empty() || path_prefix.empty()) {
		push_to_stack(-1);
		return;
	}

	int result = 0;
	try {
		set_path_prefix(configure_file, path_prefix);
	}
	catch(std::exception const & ex) {
		std::cerr << "ERROR: set_path_prefix:\n\t"
			  << ex.what() << '\n';
		result = -1;
	}

	push_to_stack(result);
}


// Creates the lyx.bat file in the LyX\bin folder.
extern "C"
void __declspec(dllexport) create_lyx_bat(HWND const /*hwndParent*/,
					  int const string_size,
					  char * const variables,
					  stack_t ** const stacktop)
{
	EXDLL_INIT();

	std::string const bin_dir = pop_from_stack();
	std::string const lang = pop_from_stack();
	if (bin_dir.empty() || lang.empty()) {
		push_to_stack(-1);
		return;
	}

	int result = 0;
	try {
		create_lyx_bat(bin_dir, lang);
	}
	catch(std::exception const & ex) {
		std::cerr << "ERROR: create_lyx_bat:\n\t"
			  << ex.what() << '\n';
		result = -1;
	}

	push_to_stack(result);
}


// Creates the reLyX.bat file in the LyX\bin folder.
extern "C"
void __declspec(dllexport) create_relyx_bat(HWND const /*hwndParent*/,
					    int const string_size,
					    char * const variables,
					    stack_t ** const stacktop)
{
	EXDLL_INIT();

	std::string const bin_dir = pop_from_stack();
	std::string const lang = pop_from_stack();
	if (bin_dir.empty() || lang.empty()) {
		push_to_stack(-1);
		return;
	}

	int result = 0;
	try {
		create_relyx_bat(bin_dir, lang);
	}
	catch(std::exception const & ex) {
		std::cerr << "ERROR: create_relyx_bat:\n\t"
			  << ex.what() << '\n';
		result = -1;
	}

	push_to_stack(result);
}


// Runs "sh configure" to generate things like lyxrc.defaults.
extern "C"
void __declspec(dllexport) run_configure(HWND const /*hwndParent*/,
					 int const string_size,
					 char * const variables,
					 stack_t ** const stacktop)
{
	EXDLL_INIT();

	std::string const configure_file = pop_from_stack();
	std::string const path_prefix = pop_from_stack();
	if (configure_file.empty() || path_prefix.empty()) {
		push_to_stack(-1);
		return;
	}

	int result = 0;
	try {
		run_configure(configure_file, path_prefix);
	}
	catch(std::exception const & ex) {
		std::cerr << "ERROR: run_configure:\n\t"
			  << ex.what() << '\n';
		result = -1;
	}

	push_to_stack(result);
}


// Set an environment variable.
extern "C"
void __declspec(dllexport) set_env(HWND const /*hwndParent*/,
				   int const string_size,
				   char * const variables,
				   stack_t ** const stacktop)
{
	EXDLL_INIT();

	std::string const var_name = pop_from_stack();
	std::string const var_value = pop_from_stack();
	if (var_name.empty() || var_value.empty()) {
		push_to_stack(-1);
		return;
	}

	// Function returns a nonzero value on success.
	int const result =
		SetEnvironmentVariableA(var_name.c_str(), var_value.c_str()) ?
		0 : -1;

	push_to_stack(result);
}
