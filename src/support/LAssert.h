// -*- C++ -*-
#ifndef LASSERT_H
#define LASSERT_H

//namespace LyX {

#ifdef ENABLE_ASSERTIONS

//template<class X, class A> inline void Assert(A assertion)
template<class A> inline void Assert(A assertion)
{
	//if (!assertion) throw X();
	if (!assertion) {
		abort();
	}
}

#ifdef HAVE_PARTIAL_SPECIALIZATION
// Not too sure if this is the correct way to specialize
template<class A> inline void Assert(A * ptr)
{
	Assert<void const *>(ptr);
}
template<> inline void Assert(void const * ptr)
{
	if (!ptr) {
		abort();
	}
}
#endif /* HAVE_PARTIAL_SPECIALIZATION  */ 

//template<class A, class E> inline void Assert(A assertion, E except)
//{
//	if (!assertion) except;
//}

#else

template<class A> inline void Assert(A /*assertion*/) {}

#endif /* ENABLE_ASSERTIONS */


//} // end of namespace LyX


#endif /* LASSERT_H */

