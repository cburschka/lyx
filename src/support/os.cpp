/**
 * \file os.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/qstring_helpers.h"
#include "support/regex.h"

#include <QDir>

#if defined(__CYGWIN__)
#include "support/os_cygwin.cpp"
#elif defined(_WIN32)
#include "support/os_win32.cpp"
#else
#include "support/os_unix.cpp"
#endif

// Static assert to break compilation on platforms where
// int/unsigned int is not 4 bytes. Added to make sure that
// e.g., the author hash is always 32-bit.
template<bool Condition> struct static_assert_helper;
template <> struct static_assert_helper<true> {};
enum {
	dummy = sizeof(static_assert_helper<sizeof(int) == 4>)
};

namespace lyx {
namespace support {
namespace os {

static string const python23(string const & binary, bool verbose = false)
{
	const string version_info = " -c 'from __future__ import print_function;import sys; print(sys.version_info[:2], end=\"\")'";
	static regex const python_reg("\\((\\d*), (\\d*)\\)");
	if (verbose)
		lyxerr << "Examining " << binary << "\n";

	// Check whether this is a python 2 or 3 binary.
	cmd_ret const out = runCommand(binary + version_info);

	smatch sm;
	if (out.first < 0 || !regex_match(out.second, sm, python_reg))
		return string();

	int major = convert<int>(sm.str(1));
	int minor = convert<int>(sm.str(2));
	if((major == 2 and minor < 7) or (major == 3 and minor < 4))
		return string();

	if (verbose)
		lyxerr << "Found Python " << out.second << "\n";
	return binary;
}


int timeout_min()
{
	return 3;
}


string const python(bool reset)
{
	// Check whether python3 in PATH is the right one.
	static string command = python23("python3");
	// FIXME THREAD
	if (reset) {
		command = python23("python3");
	}

	// python3 does not exists, let us try python3.x
	if (command.empty()) {
		// It was not, so check whether we can find it elsewhere in
		// PATH, maybe with some suffix appended.
		vector<string> const path = getEnvPath("PATH");
		lyxerr << "Looking for python 3.x ...\n";
		for (auto bin: path) {
			QString const dir = toqstr(bin);
			string const localdir = dir.toLocal8Bit().constData();
			QDir qdir(dir);
			qdir.setFilter(QDir::Files | QDir::Executable);
			QStringList list = qdir.entryList(QStringList("python3*"));
			for (int i = 0; i < list.size() && command.empty(); ++i) {
				string const binary = addName(localdir,
					list.at(i).toLocal8Bit().constData());
				command = python23(binary, true);
			}
		}

	}
	// python 3 not found let us look for python 2
	if (command.empty())
		command = python23("python2");

	// python2 does not exists, let us try python2.x
	if (command.empty()) {
		// It was not, so check whether we can find it elsewhere in
		// PATH, maybe with some suffix appended.
		vector<string> const path = getEnvPath("PATH");
		lyxerr << "Looking for python 3.x ...\n";
		for (auto bin: path) {
			QString const dir = toqstr(bin);
			string const localdir = dir.toLocal8Bit().constData();
			QDir qdir(dir);
			qdir.setFilter(QDir::Files | QDir::Executable);
			QStringList list = qdir.entryList(QStringList("python2*"));
			for (int i = 0; i < list.size() && command.empty(); ++i) {
				string const binary = addName(localdir,
					list.at(i).toLocal8Bit().constData());
				command = python23(binary, true);
			}
		}

	}

	// Default to "python" if no usable binary was found.
	// If this happens all hope is lost that there is a sane system
	if (command.empty()) {
		lyxerr << "Warning: No python v2.x or 3.x binary found.\n";
		command = "python";
	}

	// Add the -tt switch so that mixed tab/whitespace
	// indentation is an error
	command += " -tt";
	return command;
}

} // namespace os
} // namespace support
} // namespace lyx
