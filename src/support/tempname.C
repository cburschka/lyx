/**
 * \file tempname.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <cstdlib>
#include <unistd.h>

#include "LString.h"
#include "support/lyxlib.h"
#include "support/filetools.h"
#include "support/tostr.h"
#include "debug.h"
#include "os.h"

using std::endl;

namespace {

inline
int make_tempfile(char * templ)
{
#if defined(HAVE_MKSTEMP)
	return ::mkstemp(templ);
#elif defined(HAVE_MKTEMP)
	// This probably just barely works...
	::mktemp(templ);
	return ::open(templ, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
#else
#error FIX FIX FIX
#endif
}

} // namespace anon


string const lyx::support::tempName(string const & dir, string const & mask)
{
	string const tmpdir(dir.empty() ? os::getTmpDir() : dir);
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
