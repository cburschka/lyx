// os_unix.C

// Various OS specific functions
#include <config.h>

#include "os.h"

string const os::nulldev_ = "/dev/null";
os::shell_type os::shell_ = os::UNIX;


void os::init(int, char *[])
{}


string os::current_root()
{
	return string("/");
}


string::size_type os::common_path(string const &p1, string const &p2)
{
	string::size_type i = 0,
			p1_len = p1.length(),
			p2_len = p2.length();
	while (i < p1_len && i < p2_len && p1[i] == p2[i]) ++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && p1[i] != '/' && i == p2_len)
	    || (i < p2_len && p2[i] != '/' && i == p1_len)) {
		if (i) --i;     // here was the last match
		while (i && p1[i] != '/') --i;
	}
	return i;
}


string os::internal_path(string const & p)
{
	return p;
}

string os::external_path(string const & p)
{
	return p;
}


bool os::is_absolute_path(string const & p)
{
	return (!p.empty() && p[0] == '/');
}


// returns a string suitable to be passed to fopen when
// reading a file
char const * os::read_mode()
{
	return "r";
}


// returns a string suitable to be passed to popen when
// reading a pipe
char const * os::popen_read_mode()
{
	return "r";
}


// The character used to separate paths returned by the
//  PATH environment variable.
char os::path_separator()
{
	return ':';
}


void os::cygwin_path_fix(bool)
{}
