// -*- C++ -*-

#ifndef LYX_FUNCTIONAL_H
#define LYX_FUNCTIONAL_H

/** \file lyxfunctional.h
    \brief Convenient function objects for use with LyX
    
    This is currently a small collection of small function objects for use
    together with std::algorithms.
**/

#include <iterator>

namespace lyx {


template<class R, class C, class A>
class class_fun_t {
public:
	class_fun_t(C & ct, R(C::*p)(A)) 
		: c(ct), cmf(p) {}
	R operator()(A & a) const {
		return (c.*cmf)(a);
	}
private:
	C & c;
	R(C::*cmf)(A);
};


template<class C, class A>
class void_class_fun_t {
public:
	void_class_fun_t(C & ct, void(C::*p)(A))
		: c(ct), cmf(p) {}
	void operator()(A & a) const {
		(c.*cmf)(a);
	}
private:
	C & c;
	void(C::*cmf)(A);
};


/// Use to call a class method with a container element.
/** Most easily used as a functor to std::algoritms.
    Small example:
    \verbatim
    A a; // class that have a int print(string const &) method
    vector<string> vs;
    for_each(vs.begin(), vs.end(), class_fun(int, vs, &A::print);
    \endverbatim
**/
template <class R, class C, class A> class_fun_t<R, C, A>
class_fun(C & c, R(C::*f)(A))
{
	return class_fun_t<R, C, A>(c, f);
}


template <class C, class A> void_class_fun_t<C, A>
class_fun(C & c, void(C::*f)(A))
{
	return void_class_fun_t<C, A>(c, f);
}


template <class Cont, class Type, class MemRet>
class back_insert_fun_iterator {
protected:
	Cont * container;
	MemRet(Type::*pmf)();
public:
	typedef Cont container_type;
	typedef std::output_iterator_tag iterator_category;
	typedef void value_type;
	typedef void difference_type;
	typedef void pointer;
	typedef void reference;
       
	back_insert_fun_iterator(Cont & x, MemRet(Type::*p)())
		: container(&x), pmf(p) {}

	back_insert_fun_iterator &
	operator=(Type * val) {
		container->push_back((val->*pmf)());
		return *this;
	}

	back_insert_fun_iterator &
	operator=(Type & val) {
		container->push_back((val.*pmf)());
		return *this;
	}

	back_insert_fun_iterator & operator*() {
		return *this;
	}
	back_insert_fun_iterator & operator++() { // prefix ++
		return *this;
	}
	back_insert_fun_iterator & operator++(int) { // postfix ++
		return *this;
	}
};


template <class Cont, class Type, class MemRet>
class const_back_insert_fun_iterator {
protected:
	Cont * container;
	MemRet(Type::*pmf)() const;
public:
	typedef Cont container_type;
	typedef std::output_iterator_tag iterator_category;
	typedef void value_type;
	typedef void difference_type;
	typedef void pointer;
	typedef void reference;
	
	const_back_insert_fun_iterator(Cont & x, MemRet(Type::*p)() const)
		: container(&x), pmf(p) {}
	
	~const_back_insert_fun_iterator() {}
      
	const_back_insert_fun_iterator &
	operator=(Type const * val) {
		container->push_back((val->*pmf)());
		return *this;
	}

	const_back_insert_fun_iterator &
	operator=(Type const & val) {
		container->push_back((val.*pmf)());
		return *this;
	}

	const_back_insert_fun_iterator & operator*() {
		return *this;
	}
	const_back_insert_fun_iterator & operator++() { // prefix ++
		return *this;
	}
	const_back_insert_fun_iterator & operator++(int) { // postfix ++
		return *this;
	}
};


template <class Cont, class Type, class MemRet>
back_insert_fun_iterator<Cont, Type, MemRet>
back_inserter_fun(Cont & cont, MemRet(Type::*p)())
{
	return back_insert_fun_iterator<Cont, Type, MemRet>(cont, p);
}


template <class Cont, class Type, class MemRet>
const_back_insert_fun_iterator<Cont, Type, MemRet>
back_inserter_fun(Cont & cont, MemRet(Type::*p)() const)
{
	return const_back_insert_fun_iterator<Cont, Type, MemRet>(cont, p);
}


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
