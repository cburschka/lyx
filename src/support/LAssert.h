// -*- C++ -*-
#ifndef LASSERT_H
#define LASSERT_H

#include "support/lyxlib.h"

//namespace LyX {

#ifdef ENABLE_ASSERTIONS

//template<class X, class A> inline void Assert(A assertion)
template<class A> inline void Assert(A assertion)
{
	//if (!assertion) throw X();
	if (!assertion) {
		lyx::abort();
	}
}


//template<class A, class E> inline void Assert(A assertion, E except)
//{
//	if (!assertion) except;
//}

#else

template<class A> inline void Assert(A /*assertion*/) {}

#endif /* ENABLE_ASSERTIONS */


//} // end of namespace LyX


#endif /* LASSERT_H */

