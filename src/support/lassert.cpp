/**
 * \file lassert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/assert.hpp>


//#define LYX_CALLSTACK_PRINTING
// must be linked with -rdynamic
#ifdef LYX_CALLSTACK_PRINTING
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>
#endif


namespace lyx {

using namespace std;
using namespace support;

// TODO Should we try to print the call stack in the course of these?

void doAssert(char const * expr, char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	BOOST_ASSERT(false);
}


docstring formatHelper(docstring const & msg,
	char const * expr, char const * file, long line)
{
	static const docstring d = 
		from_ascii(N_("Assertion %1$s violated in\nfile: %2$s, line: %3$s"));
	
	return bformat(d, from_ascii(expr), from_ascii(file), 
		convert<docstring>(line)) + '\n' + msg;
}


void doWarnIf(char const * expr, docstring const & msg, char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	BOOST_ASSERT(false);
	throw ExceptionMessage(WarningException, _("Warning!"), 
		formatHelper(msg, expr, file, line));
}


void doBufErr(char const * expr, docstring const & msg, char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	BOOST_ASSERT(false);
	throw ExceptionMessage(BufferException, _("Buffer Error!"),
		formatHelper(msg, expr, file, line));
}


void doAppErr(char const * expr, docstring const & msg, char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	BOOST_ASSERT(false);
	throw ExceptionMessage(ErrorException, _("Fatal Exception!"),
		formatHelper(msg, expr, file, line));
}


//TODO Return as string, so call stack could be used in dialogs.
void printCallStack()
{
#ifdef LYX_CALLSTACK_PRINTING
	const int depth = 50;
	
	// get void*'s for all entries on the stack
	void* array[depth];
	size_t size = backtrace(array, depth);
	
	char** messages = backtrace_symbols(array, size);
	
	for (size_t i = 0; i < size && messages != NULL; i++) {
		std::string orig(messages[i]);
		// extract mangled: bin/lyx2.0(_ZN3lyx7support7packageEv+0x32) [0x8a2e02b]
		char* mangled = 0;
		for (char *p = messages[i]; *p; ++p) {
			if (*p == '(') {
				*p = 0;
				mangled = p + 1;
			} else if (*p == '+') {
				*p = 0;
				break;
			}
		}
		int err = 0;
		char* demangled = abi::__cxa_demangle(mangled, 0, 0, &err);
		if (err == 0) {
			fprintf(stderr, "[bt]: (%d) %s %s\n", i, messages[i], demangled);
			free((void*)demangled);
		} else {
			fprintf(stderr, "[bt]: (%d) %s\n", i, orig.c_str());
		}		
	}
#endif
}

} // namespace lyx
