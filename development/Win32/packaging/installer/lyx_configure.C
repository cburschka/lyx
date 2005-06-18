/*
 * \file lyx_configure.C
 * This file is part of LyX, the document processor.
 * http://www.lyx.org/
 * Licence details can be found in the file COPYING or copy at
 * http://www.lyx.org/about/license.php3

 * \author Angus Leeming
 * Full author contact details are available in file CREDITS or copy at
 * http://www.lyx.org/about/credits.php
 *
 * Define four functions that can be called from the NSIS installer:
 *
 * set_path_prefix [ configure_file, path_prefix ]
 * create_bat_files [ bin_dir, lang ]
 * run_configure [ configure_file, path_prefix ]
 * set_env [ var_name, var_value ]
 *
 * The quantities in [ ... ] are the variables that the functions exxpect
 * to find on the stack. They push "-1" onto the stack on failure and "0"
 * onto the stack on success.
 *
 * Compile the code with
 *
 * g++ -I/c/Program\ Files/NSIS/Contrib -Wall -shared \
 *    lyx_configure.C -o lyx_configure.dll
 *
 * Move resulting .dll to /c/Program\ Files/NSIS/Plugins
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

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
	std::string::size_type const final_slash = path.find_last_of('\\');
	return (final_slash == std::string::npos) ?
		path :
		path.substr(final_slash+1);
}


std::string const dirname(std::string const & path)
{
	std::string::size_type const final_slash = path.find_last_of('\\');
	return (final_slash == std::string::npos) ?
		std::string() :
		path.substr(0, final_slash);
}


std::string const pop_from_stack()
{
	char data[10*MAX_PATH];
	popstring(data);
	return data;
}


void push_to_stack(int data)
{
	std::ostringstream os;
	os << data;
	pushstring(os.str().c_str());
}


std::list<std::string> const tokenize(std::string data,
				      char const separator)
{
	std::list<std::string> result;
	while (true) {
		std::string::size_type const end = data.find(separator);
		if (end == std::string::npos) {
			result.push_back(data);
			break;
		}
		result.push_back(data.substr(0, end));
		data = data.substr(end+1);
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


std::string const sanitize_path(std::string const & in)
{
	// Replace multiple, adjacent directory separators.
	std::string out = subst(in, "\\\\", "\\");
	std::list<std::string> out_list = tokenize(out, ';');
	remove_duplicates(out_list);
	return concatenate(out_list, ';');
}


bool replace_path_prefix(std::string & data,
			 std::string::size_type prefix_pos,
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
	std::string const prefix =
		subst(sanitize_path(path_prefix + ';' + old_prefix), "\\", "\\\\");

	data.erase(start_prefix, count);
	data.insert(start_prefix, prefix);

	return true;
}


bool insert_path_prefix(std::string & data,
			std::string::size_type xfonts_pos,
			std::string const & path_prefix)
{
	std::string::size_type const xfonts_start =
		data.find_last_of('\n', xfonts_pos);

	if (xfonts_start == std::string::npos)
		return false;

	std::string const prefix = subst(sanitize_path(path_prefix), "\\", "\\\\");
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


// Inserts code into "configure" to output "path_prefix" to lyxrc.defaults.
// \returns 0 on success, -1 on failure
int set_path_prefix(std::string const & configure_file,
		    std::string const & path_prefix)
{
	std::ifstream ifs(configure_file.c_str());
	if (!ifs)
		return -1;

	std::istreambuf_iterator<char> const begin_ifs(ifs);
	std::istreambuf_iterator<char> const end_ifs;
	std::string configure_data(begin_ifs, end_ifs);
	ifs.close();

	// Does configure already contain a "path_prefix" entry
	// or should we insert one?
	std::string::size_type const prefix_pos =
		configure_data.find("path_prefix");
	if (prefix_pos != std::string::npos) {
		if (!replace_path_prefix(configure_data, prefix_pos, path_prefix))
			return -1;
	} else {
		std::string::size_type const xfonts_pos =
			configure_data.find("X FONTS");

		if (xfonts_pos == std::string::npos)
			return -1;

		if (!insert_path_prefix(configure_data, xfonts_pos, path_prefix))
			return -1;
	}

	std::ofstream ofs(configure_file.c_str());
	if (!ofs)
		return -1;

	ofs << configure_data;
	return 0;
}


bool write_bat(std::ostream & os, std::string const & quoted_exe)
{
	os << "if \"%~1\" == \"~1\" got win95\n"
	   << quoted_exe << " %~*\n"
	   << "goto end\n"
	   << ":win95\n"
	   << quoted_exe << " %1 %2 %3 %4 %5 %6 %7 %8 %9\n"
	   << ":end\n";

	return os;
}


// Creates the files lyx.bat and reLyX.bat in the LyX\bin folder.
// \returns 0 on success, -1 on failure
int create_bat_files(std::string const & bin_dir, std::string const & lang)
{
	std::string const lyx_bat_file = bin_dir + "\\lyx.bat";
	std::ofstream lyx_bat(lyx_bat_file.c_str());
	if (!lyx_bat)
		return -1;

	lyx_bat << "@echo off\n"
		<< "if \"%LANG%\"==\"\" SET LANG=" << lang << "\n";

	std::string const lyx_exe_file = bin_dir + "\\lyx.exe";
	if (!write_bat(lyx_bat, "\"" + lyx_exe_file + "\""))
		return -1;

	std::string const relyx_bat_file = bin_dir + "\\reLyX.bat";
	std::ofstream relyx_bat(relyx_bat_file.c_str());
	if (!relyx_bat)
		return -1;

	std::string relyx_file = bin_dir + "\\reLyX";
	std::string relyx = "perl.exe \"" + relyx_file + "\"";
	if (!write_bat(relyx_bat, "perl.exe \"" + relyx_file + "\""))
		return -1;
	return 0;
}


// Runs "sh configure" to generate things like lyxrc.defaults.
// \returns 0 on success, -1 on failure
int run_configure(std::string const & abs_configure_file,
		  std::string const & path_prefix)
{
	std::string const configure_dir = dirname(abs_configure_file);
	std::string const configure_file = basename(abs_configure_file);

	if (configure_dir.empty())
		return -1;

	if (SetCurrentDirectory(configure_dir.c_str()) == 0)
		return -1;

	char path_orig[10*MAX_PATH];
	if (GetEnvironmentVariable("PATH", path_orig, 10*MAX_PATH) == 0)
		return -1;

	std::string const path = path_prefix + ';' + path_orig;
	if (SetEnvironmentVariable("PATH", path.c_str()) == 0)
		return -1;

	// Even "start /WAIT /B sh.exe configure" returns
	// before the script is done, so just invoke "sh" directly.
	std::string const command = std::string("sh.exe ") + configure_file;
	if (system(command.c_str()) != 0)
		return -1;
	return 0;
}

} // namespace anon


//===========================================//
//                                           //
// Below is the public interface to the .dll //
//                                           //
//===========================================//

BOOL WINAPI DllMain(HANDLE hInst,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
{
	return TRUE;
}


// Inserts code into "configure" to output "path_prefix" to lyxrc.defaults.
extern "C"
void __declspec(dllexport) set_path_prefix(HWND hwndParent,
					   int string_size,
					   char * variables,
					   stack_t ** stacktop)
{
	EXDLL_INIT();

	std::string const configure_file = pop_from_stack();
	std::string const path_prefix = pop_from_stack();

	int const result = set_path_prefix(configure_file, path_prefix);
	push_to_stack(result);
}


// Creates the files lyx.bat and reLyX.bat in the LyX\bin folder.
extern "C"
void __declspec(dllexport) create_bat_files(HWND hwndParent,
					    int string_size,
					    char * variables,
					    stack_t ** stacktop)
{
	EXDLL_INIT();

	std::string const bin_dir = pop_from_stack();
	std::string const lang = pop_from_stack();

	int const result = create_bat_files(bin_dir, lang);
	push_to_stack(result);
}


// Runs "sh configure" to generate things like lyxrc.defaults.
extern "C"
void __declspec(dllexport) run_configure(HWND hwndParent,
					 int string_size,
					 char * variables,
					 stack_t ** stacktop)
{
	EXDLL_INIT();

	std::string const configure_file = pop_from_stack();
	std::string const path_prefix = pop_from_stack();

	int const result = run_configure(configure_file, path_prefix);
	push_to_stack(result);
}


// Set an environment variable
extern "C"
void __declspec(dllexport) set_env(HWND hwndParent,
				   int string_size,
				   char * variables,
				   stack_t ** stacktop)
{
	EXDLL_INIT();

	std::string const var_name = pop_from_stack();
	std::string const var_value = pop_from_stack();

	// Function returns a nonzero value on success.
	int const result =
		SetEnvironmentVariableA(var_name.c_str(), var_value.c_str()) ?
		0 : -1;

	push_to_stack(result);
}
