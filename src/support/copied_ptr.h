// -*- C++ -*-
/**
 * \file copied_ptr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * A templated class that can serve as a pointer to an object, with the
 * property that when the copied_ptr is copied, it creates its own copy
 * of the object pointed to as well.
 *
 * The implementation was based originally on Yonat Sharon's copied_ptr templated
 * class, as described at http://ootips.org/yonat/, but has evolved toward's
 * Herb Sutter's HolderPtr, as described at http://www.gotw.ca/gotw/062.htm.
 * (Note, HolderPtr became ValuePtr in his book, More Exceptional C++.)
 *
 * Warning: if the class stores 'Base * ptr_', but the actual object pointed to
 * belongs to a derived class, then you must specialise memory_traits<Base> so that
 * its clone and destroy member functions do the right thing. Otherwise, you'll
 * end up slicing the data.
 */

#ifndef COPIED_PTR_H
#define COPIED_PTR_H

namespace lyx {
namespace support {

template <typename T>
struct memory_traits {
	static T * clone(T const * ptr) { return new T(*ptr); }
	static void destroy(T * ptr) { delete ptr; }
};


template <typename T, typename Traits=memory_traits<T> >
class copied_ptr {
public:
	explicit copied_ptr(T * = 0);
	copied_ptr(copied_ptr const &);
	~copied_ptr();
	copied_ptr & operator=(copied_ptr const &);

	T & operator*() const;
	T * operator->() const;
	T * get() const;

private:
	T * ptr_;
};


template <typename T, typename Traits>
copied_ptr<T, Traits>::copied_ptr(T * p)
	: ptr_(p)
{}


template <typename T, typename Traits>
copied_ptr<T, Traits>::copied_ptr(copied_ptr const & other)
	: ptr_(other.ptr_ ? Traits::clone(other.ptr_) : 0)
{}


template <typename T, typename Traits>
copied_ptr<T, Traits>::~copied_ptr()
{
	Traits::destroy(ptr_);
}


template <typename T, typename Traits>
copied_ptr<T, Traits> & copied_ptr<T, Traits>::operator=(copied_ptr const & other)
{
	if (&other != this) {
		copied_ptr temp(other);
		std::swap(ptr_, temp.ptr_);
	}
	return *this;
}


template <typename T, typename Traits>
T & copied_ptr<T, Traits>::operator*() const
{
	return *ptr_;
}


template <typename T, typename Traits>
T * copied_ptr<T, Traits>::operator->() const
{
	return ptr_;
}


template <typename T, typename Traits>
T * copied_ptr<T, Traits>::get() const
{
	return ptr_;
}

} // namespace support
} // namespace lyx

#endif // NOT COPIED_PTR_H
