#include <config.h>

#include <cstdlib>
#include <unistd.h>

#include "LString.h"
#include "support/lyxlib.h"
#include "support/filetools.h"
#include "debug.h"

using std::endl;

extern string system_tempdir;

namespace {

inline
int make_tempfile(char * templ) 
{
#ifdef HAVE_MKSTEMP
	return ::mkstemp(templ);
#else
#ifdef HAVE_MKTEMP
	// This probably just barely works...
	::mktemp(templ);
	return ::open(templ, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
#else
#warning FIX FIX FIX
#endif
#endif
}

} // namespace anon


string const lyx::tempName(string const & dir, string const & mask)
{
	string const tmpdir(dir.empty() ? system_tempdir : dir);
	string tmpfl(AddName(tmpdir, mask));
	tmpfl += tostr(getpid());
	tmpfl += "XXXXXX";

	// The supposedly safe mkstemp version
	char * tmpl = new char[tmpfl.length() + 1]; // + 1 for '\0'
	tmpfl.copy(tmpl, string::npos);
	tmpl[tmpfl.length()] = '\0'; // terminator
	
	int const tmpf = make_tempfile(tmpl);
	if (tmpf != -1) {
		string const t(tmpl);
		::close(tmpf);
		delete [] tmpl;
		lyxerr[Debug::FILES] << "Temporary file `" << t
				     << "' created." << endl;
		return t;
	} else {
		lyxerr[Debug::FILES]
			<< "LyX Error: Unable to create temporary file."
			<< endl;
		delete [] tmpl;
		return string();
	}
}
