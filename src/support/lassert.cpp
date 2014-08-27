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
#include <lassert.h>

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/assert.hpp>

#include <QString>

#ifdef LYX_CALLSTACK_PRINTING
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>
#endif


namespace lyx {

using namespace std;
using namespace support;


void doAssertWithCallstack(bool value)
{
	if (!value) {
		printCallStack();
		BOOST_ASSERT(false);
	}
}


void doAssert(char const * expr, char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	doAssertWithCallstack(false);
}


docstring formatHelper(docstring const & msg,
	char const * expr, char const * file, long line)
{
	docstring const d = _("Assertion %1$s violated in\nfile: %2$s, line: %3$s");
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	
	return bformat(d, from_ascii(expr), from_ascii(file), 
		convert<docstring>(line)) + '\n' + msg;
}


void doWarnIf(char const * expr, char const * file, long line)
{
	docstring const d = _("It should be safe to continue, but you\nmay wish to save your work and restart LyX.");
	// comment this out if not needed
	doAssertWithCallstack(false);
	throw ExceptionMessage(WarningException, _("Warning!"), 
		formatHelper(d, expr, file, line));
}


void doBufErr(char const * expr, char const * file, long line)
{
	docstring const d = _("There has been an error with this document.\nLyX will attempt to close it safely.");
	// comment this out if not needed
	doAssertWithCallstack(false);
	throw ExceptionMessage(BufferException, _("Buffer Error!"),
		formatHelper(d, expr, file, line));
}


void doAppErr(char const * expr, char const * file, long line)
{
	docstring const d = _("LyX has encountered an application error\nand will now shut down.");
	// comment this out if not needed
	doAssertWithCallstack(false);
	throw ExceptionMessage(ErrorException, _("Fatal Exception!"),
		formatHelper(d, expr, file, line));
}


docstring printCallStack()
{
#ifndef LYX_CALLSTACK_PRINTING
	return docstring();
#else
	const int depth = 200;
	
	// get void*'s for all entries on the stack
	void* array[depth];
	size_t size = backtrace(array, depth);
	
	char** messages = backtrace_symbols(array, size);
	
	docstring bt;
	for (size_t i = 1; i < size && messages != NULL; i++) {
		const std::string orig(messages[i]);
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
		int status = 0;
		const char* demangled = abi::__cxa_demangle(mangled, 0, 0, &status);
		const QByteArray line = QString("(%1) %2: %3\n").arg(i, 3).arg(messages[i])
								.arg(demangled ? demangled : orig.c_str()).toLocal8Bit();
		free((void*)demangled);

		fprintf(stderr, "%s", line.constData());
		bt += from_local8bit(line.constData());
	}
		return bt;
#endif
}

} // namespace lyx
