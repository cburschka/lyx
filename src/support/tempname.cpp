/**
 * \file tempname.cpp
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
#include "support/Package.h"

#include "debug.h"

#include <boost/scoped_array.hpp>

#include <cstdlib>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(HAVE_MKSTEMP) && ! defined(HAVE_DECL_MKSTEMP)
extern "C" int mkstemp(char *);
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

namespace lyx {
namespace support {

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
# if (!defined S_IRUSR)
#   define S_IRUSR S_IREAD
#   define S_IWUSR S_IWRITE
# endif
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


FileName const tempName(FileName const & dir, string const & mask)
{
	string const tmpdir(dir.empty() ?
			package().temp_dir().absFilename() :
			dir.absFilename());
	string tmpfl(to_filesystem8bit(from_utf8(addName(tmpdir, mask))));
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
		string const t(to_utf8(from_filesystem8bit(tmpl.get())));
#if defined (HAVE_CLOSE)
		::close(tmpf);
#elif defined (HAVE__CLOSE)
		::_close(tmpf);
#else
# error No x() function.
#endif
		LYXERR(Debug::FILES) << "Temporary file `" << t
				     << "' created." << endl;
		return FileName(t);
	} else {
		LYXERR(Debug::FILES)
			<< "LyX Error: Unable to create temporary file."
			<< endl;
		return FileName();
	}
}

} // namespace support
} // namespace lyx
