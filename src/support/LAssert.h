// -*- C++ -*-
#ifndef LASSERT_H
#define LASSERT_H

#include "support/lyxlib.h"

extern void emergencyCleanup();

namespace lyx {

#ifdef ENABLE_ASSERTIONS

/** Live assertion.
    This is a debug tool to ensure that the assertion holds. If it don't hole
    we run #emergencyCleanup()# and then #lyx::abort".
    @param assertion this should evaluate to true unless you want an abort.
*/
template<class A>
inline
void Assert(A assertion)
{
	if (!assertion) {
		::emergencyCleanup();
		lyx::abort();
	}
}

#else

/** Dummy assertion.
    When compiling without assertions we use this no-op function.
*/
template<class A>
inline
void Assert(A /*assertion*/) {}

#endif /* ENABLE_ASSERTIONS */

} // end of namespace LyX

#endif /* LASSERT_H */
