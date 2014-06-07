
/**
 * \file support/lassert.h
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LASSERT_H
#define LASSERT_H

#ifdef __cplusplus

#include "support/strfwd.h"

namespace lyx {

/******************************************************************************

LyX has five different macros that can be used to make assertions. They behave
the same way in devel mode: They assert. The differences between them are how
they behave in release mode.

In order of increasing seriousness, they are:

LATTEST(expr)
  This macro should be used when one just wants to test expr. If devel mode,
  this will lead to an assertion. In release mode, we will simply continue. So
  LATTEST should be used only if you know, in advance, that it will be safe to
  continue with the usual program flow, but failure of expr still means that 
  there is something that needs to be fixed.

LASSERT(expr, escape)
  This macro should be used when a failure of expr is not compatible with 
  continuing the ordinary program flow, but is something from which we can
  recover. This might mean simply returning early from some routine; it might
  mean resetting some variables to values known to be sane; it might mean
  taking some other corrective action. 

LWARNIF(expr)
  This macro should be used when a failure of expr indicates that the current
	operation cannot safely be completed. In release mode, it will abort that
	operation and print a warning message to the user.

LBUFERR(expr)
  This macro should be used when a failure of expr indicates a problem with a
	Buffer or its related objects, e.g., a Cursor. In release mode, it throws a
	BufferException, which will typically result in an emergency save of that
	particular Buffer.

LAPPERR(expr)
  This macro should be used if a failure of expr is incompatible with LyX
	continuing to operate at all. In release mode, this issues an ErrorException,
  which typically results in an emergency shutdown.
	
******************************************************************************/


void doAssert(char const * expr, char const * file, long line);
void doWarnIf(char const * expr, char const * file, long line);
void doBufErr(char const * expr, char const * file, long line);
void doAppErr(char const * expr, char const * file, long line);

/// Print demangled callstack to stderr
docstring printCallStack();


} // namespace lyx

#define LATTEST(expr) \
	if (expr) {} else { lyx::doAssert(#expr, __FILE__, __LINE__); }

#define LASSERT(expr, escape) \
	if (expr) {} else { lyx::doAssert(#expr, __FILE__, __LINE__); escape; }

#define LWARNIF(expr) \
	if (expr) {} else { lyx::doWarnIf(#expr, __FILE__, __LINE__); }

#define LBUFERR(expr) \
	if (expr) {} else { lyx::doBufErr(#expr, __FILE__, __LINE__); }

#define LAPPERR(expr) \
	if (expr) {} else { lyx::doAppErr(#expr, __FILE__, __LINE__); }

#endif
#endif // LASSERT
