#include <config.h>

#include "LString.h"

#include <cstdlib>

#include <unistd.h>

#include "lyxlib.h"
#include "debug.h"
#include "filetools.h"

using std::endl;

extern string system_tempdir;

string const lyx::tempName(string const & dir, string const & mask)
{
#if 0
	// the tmpnam version...
	char const * const tmp = ::tmpnam(0);
	return (tmp) ? tmp : string ();
#else
	string tmpdir;
	if (dir.empty())
		tmpdir = system_tempdir;
	else
		tmpdir = dir;
	string tmpfl(AddName(tmpdir, mask));
	tmpfl += tostr(getpid());

	// the supposedly safer mkstemp version
	char * tmpl = new char[256];
	tmpfl += ".XXXXXX";
	::strcpy(tmpl, tmpfl.c_str());
	int tmpf = ::mkstemp(tmpl);
	if (tmpf != -1) {
		string const t(tmpl);
		::close(tmpf);
		delete [] tmpl;
		lyxerr << "Temporary file `" << t << "' created." << endl;
		return t;
	} else {
		lyxerr << "LyX Error: Unable to create temporary file."
		       << endl;
		delete [] tmpl;
		return string();
	}
#endif
}
