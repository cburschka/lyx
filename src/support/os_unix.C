/**
 * \file os_unix.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 *
 * Various OS specific functions
 */

#include <config.h>

#include "os.h"
#include "support/filetools.h"
#include "support/lstrings.h"


using std::string;


namespace {

string binpath_;
string binname_;
string tmpdir_;

}

namespace lyx {
namespace support {
namespace os {

void init(int * /*argc*/, char ** argv[])
{
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	string tmp = *argv[0];
	binname_ = OnlyFilename(tmp);
	tmp = ExpandPath(tmp); // This expands ./ and ~/
	if (!is_absolute_path(tmp)) {
		string binsearchpath = GetEnvPath("PATH");
		// This will make "src/lyx" work always :-)
		binsearchpath += ";.";
		tmp = FileOpenSearch(binsearchpath, tmp);
	}

	tmp = MakeAbsPath(OnlyPath(tmp));

	// In case we are running in place and compiled with shared libraries
	if (suffixIs(tmp, "/.libs/"))
		tmp.erase(tmp.length() - 6, string::npos);
	binpath_ = tmp;
}


void warn(string const & /*mesg*/)
{
	return;
}


string current_root()
{
	return "/";
}


string::size_type common_path(string const & p1, string const & p2)
{
	string::size_type i = 0;
	string::size_type p1_len = p1.length();
	string::size_type p2_len = p2.length();
	while (i < p1_len && i < p2_len && p1[i] == p2[i])
		++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && p1[i] != '/' && i == p2_len)
	    || (i < p2_len && p2[i] != '/' && i == p1_len))
	{
		if (i)
			--i;     // here was the last match
		while (i && p1[i] != '/')
			--i;
	}
	return i;
}


string slashify_path(string const & p)
{
	return p;
}


string external_path(string const & p)
{
	return p;
}


string internal_path(string const & p)
{
	return p;
}


bool is_absolute_path(string const & p)
{
	return !p.empty() && p[0] == '/';
}


char const * popen_read_mode()
{
	return "r";
}


string binpath()
{
	return binpath_;
}


string binname()
{
	return binname_;
}


void setTmpDir(string const & p)
{
	tmpdir_ = p;
}


string getTmpDir()
{
	return tmpdir_;
}


shell_type shell()
{
	return UNIX;
}

} // namespace os
} // namespace support
} // namespace lyx
