// -*- C++ -*-
/**
 * \file lyxfunctional.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * \brief Convenient function objects for use with LyX
 *
 * This is currently a small collection of small function objects for use
 * together with std::algorithms.
 */


#ifndef LYX_FUNCTIONAL_H
#define LYX_FUNCTIONAL_H

#include <iterator>

namespace lyx {

template <class R, class C, class A>
class compare_memfun_t {
public:
	compare_memfun_t(R(C::*p)(), A const & a)
		: pmf(p), arg(a) {}
	bool operator()(C * c) {
		return (c->*pmf)() == arg;
	}
	bool operator()(C & c) {
		return (c.*pmf)() == arg;
	}
private:
	R(C::*pmf)();
	A const & arg;
};


template <class R, class C, class A>
class const_compare_memfun_t {
public:
	const_compare_memfun_t(R(C::*p)() const, A const & a)
		: pmf(p), arg(a) {}
	bool operator()(C const * c) {
		return (c->*pmf)() == arg;
	}
	bool operator()(C const & c) {
		return (c.*pmf)() == arg;
	}
private:
	R(C::*pmf)() const;
	A const & arg;
};


template <class R, class C, class A>
compare_memfun_t<R, C, A>
compare_memfun(R(C::*p)(), A const & a)
{
	return compare_memfun_t<R, C, A>(p, a);
}


template <class R, class C, class A>
const_compare_memfun_t<R, C, A>
compare_memfun(R(C::*p)() const, A const & a)
{
	return const_compare_memfun_t<R, C, A>(p, a);
}


// Functors used in the template.

///
template<typename T>
class equal_1st_in_pair {
public:
	///
	typedef typename T::first_type first_type;
	///
	typedef T pair_type;
	///
	equal_1st_in_pair(first_type const & value) : value_(value) {}
	///
	bool operator() (pair_type const & p) const {
		return p.first == value_;
	}
private:
	///
	first_type const & value_;
};


///
template<typename T>
class equal_2nd_in_pair {
public:
	///
	typedef typename T::second_type second_type;
	///
	typedef T pair_type;
	///
	equal_2nd_in_pair(second_type const & value) : value_(value) {}
	///
	bool operator() (pair_type const & p) const {
		return p.second == value_;
	}
private:
	///
	second_type const & value_;
};

}  // end of namespace lyx
#endif
