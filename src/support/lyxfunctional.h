// -*- C++ -*-

#ifndef LYX_FUNCTIONAL_H
#define LYX_FUNCTIONAL_H

//namespace lyx {

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
	Cont & container;
	MemRet(Type::*pmf)();
public:
	back_insert_fun_iterator(Cont & x, MemRet(Type::*p)())
		: container(x), pmf(p) {}

	back_insert_fun_iterator &
	operator=(Type * val) {
		container.push_back((val->*pmf)());
		return *this;
	}

	back_insert_fun_iterator &
	operator=(Type & val) {
		container.push_back((val.*pmf)());
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
	Cont & container;
	MemRet(Type::*pmf)() const;
public:
	const_back_insert_fun_iterator(Cont & x, MemRet(Type::*p)() const)
		: container(x), pmf(p) {}

	const_back_insert_fun_iterator &
	operator=(Type const * val) {
		container.push_back((val->*pmf)());
		return *this;
	}

	const_back_insert_fun_iterator &
	operator=(Type const & val) {
		container.push_back((val.*pmf)());
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
template<typename T1, typename T2>
class equal_1st_in_pair {
public:
	///
	equal_1st_in_pair(T1 const & value) : value_(value) {}
	///
	typedef std::pair<T1, T2> pair_type;
	///
	bool operator() (pair_type const & p) const {
		return p.first == value_;
	}
private:
	///
	T1 const & value_;
};


///
template<typename T1, typename T2>
class equal_2nd_in_pair {
public:
	///
	equal_2nd_in_pair(T2 const & value) : value_(value) {}
	///
	typedef std::pair<T1, T2> pair_type;
	///
	bool operator() (pair_type const & p) const {
		return p.second == value_;
	}
private:
	///
	T2 const & value_;
};


// }  // end of namespace lyx
#endif
