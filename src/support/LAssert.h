// -*- C++ -*-
#ifndef LASSERT_H
#define LASSERT_H

#include "support/lyxlib.h"

//namespace LyX {

#ifdef ENABLE_ASSERTIONS

template<class A> inline void Assert(A assertion)
{
	if (!assertion) {
		lyx::abort();
	}
}

#else

template<class A> inline void Assert(A /*assertion*/) {}

#endif /* ENABLE_ASSERTIONS */

//} // end of namespace LyX
#endif /* LASSERT_H */

