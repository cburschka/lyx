/**
 * \file tempname.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/package.h"

#include "debug.h"

#include <boost/scoped_array.hpp>

#include <cstdlib>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if !defined(HAVE_MKSTEMP) && defined(HAVE_MKTEMP)
# include <fcntl.h>
# ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif
# ifdef HAVE_IO_H
#  include <io.h>
# endif
# ifdef HAVE_PROCESS_H
#  include <process.h>
# endif
#endif

using boost::scoped_array;

using std::string;
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
# if defined (HAVE_OPEN)
	return ::open(templ, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
# elif defined (HAVE__OPEN)
	return ::_open(templ,
	               _O_RDWR | _O_CREAT | _O_EXCL,
	               _S_IREAD | _S_IWRITE);
# else
#  error No open() function.
# endif
#else
#error FIX FIX FIX
#endif
}

} // namespace anon


string const lyx::support::tempName(string const & dir, string const & mask)
{
	string const tmpdir(dir.empty() ? package().temp_dir() : dir);
	string tmpfl(AddName(tmpdir, mask));
#if defined (HAVE_GETPID)
	tmpfl += convert<string>(getpid());
#elif defined (HAVE__GETPID)
	tmpfl += convert<string>(_getpid());
#else
# error No getpid() function
#endif
	tmpfl += "XXXXXX";

	// The supposedly safe mkstemp version
	scoped_array<char> tmpl(new char[tmpfl.length() + 1]); // + 1 for '\0'
	tmpfl.copy(tmpl.get(), string::npos);
	tmpl[tmpfl.length()] = '\0'; // terminator

	int const tmpf = make_tempfile(tmpl.get());
	if (tmpf != -1) {
		string const t(tmpl.get());
#if defined (HAVE_CLOSE)
		::close(tmpf);
#elif defined (HAVE__CLOSE)
		::_close(tmpf);
#else
# error No close() function.
#endif
		lyxerr[Debug::FILES] << "Temporary file `" << t
				     << "' created." << endl;
		return t;
	} else {
		lyxerr[Debug::FILES]
			<< "LyX Error: Unable to create temporary file."
			<< endl;
		return string();
	}
}
