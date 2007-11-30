// -*- C++ -*-
/**
 * \file cow_ptr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * A pointer with copy-on-write semantics
 *
 * The original version of this class was written by Yonat Sharon
 * and is freely available at http://ootips.org/yonat/
 *
 * I modified it to use boost::shared_ptr internally, rather than use his
 * home-grown equivalent.
 */

#ifndef COW_PTR_H
#define COW_PTR_H

#include <boost/shared_ptr.hpp>


namespace lyx {
namespace support {

template <typename T>
class cow_ptr {
public:
	explicit cow_ptr(T * = 0);
	cow_ptr(cow_ptr const &);
	cow_ptr & operator=(cow_ptr const &);

	T const & operator*() const;
	T const * operator->() const;
	T const * get() const;

	T & operator*();
	T * operator->();
	T * get();

private:
	boost::shared_ptr<T> ptr_;
	void copy();
};


template <typename T>
cow_ptr<T>::cow_ptr(T * p)
	: ptr_(p)
{}


template <typename T>
cow_ptr<T>::cow_ptr(cow_ptr const & other)
	: ptr_(other.ptr_)
{}


template <typename T>
cow_ptr<T> & cow_ptr<T>::operator=(cow_ptr const & other)
{
	if (&other != this)
		ptr_ = other.ptr_;
	return *this;
}


template <typename T>
T const & cow_ptr<T>::operator*() const
{
	return *ptr_;
}


template <typename T>
T const * cow_ptr<T>::operator->() const
{
	return ptr_.get();
}


template <typename T>
T const * cow_ptr<T>::get() const
{
	return ptr_.get();
}


template <typename T>
T & cow_ptr<T>::operator*()
{
	copy();
	return *ptr_;
}


template <typename T>
T * cow_ptr<T>::operator->()
{
	copy();
	return ptr_.get();
}


template <typename T>
T * cow_ptr<T>::get()
{
	copy();
	return ptr_.get();
}


template <typename T>
void cow_ptr<T>::copy()
{
	if (!ptr_.unique())
		ptr_ = boost::shared_ptr<T>(new T(*ptr_.get()));
}

} // namespace support
} // namespace lyx

#endif // NOT COW_PTR_H
