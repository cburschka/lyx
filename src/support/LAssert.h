// -*- C++ -*-
#ifndef LASSERT_H
#define LASSERT_H

//namespace LyX {

#define HAVE_TEMPLATE
#ifdef HAVE_TEMPLATE
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
	if (!ptr) {
		abort();
	}
}
#endif

//template<class A, class E> inline void Assert(A assertion, E except)
//{
//	if (!assertion) except;
//}

#else

inline void lyx_assert(...)
{
	// nothing
}

#endif
#endif

//} // end of namespace LyX
