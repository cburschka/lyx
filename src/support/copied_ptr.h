// -*- C++ -*-
/**
 * \file copied_ptr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yonat Sharon http://ootips.org/yonat/
 *
 * simple copy-on-create/assign pointer.
 *
 * Note: If the actual object pointed to belongs to a derived class,
 * then copied_ptr will not create a copy of the derived class object,
 * but a new base class object.
 * If you want to use a polymorphic copy-on-assign pointer, use
 * cloned_ptr.
 */

#ifndef COPIED_PTR_H
#define COPIED_PTR_H

namespace lyx {
namespace support {

template <typename T>
class copied_ptr {
public:
	explicit copied_ptr(T * = 0);
	~copied_ptr();
	copied_ptr(copied_ptr const &);
	copied_ptr & operator=(copied_ptr const &);

	T & operator*() const;
	T * operator->() const;
	T * get() const;

private:
	T * ptr_;
	void copy(copied_ptr const &);
};


template <typename T>
copied_ptr<T>::copied_ptr(T * p)
	: ptr_(p)
{}


template <typename T>
copied_ptr<T>::~copied_ptr()
{
	delete ptr_;
}


template <typename T>
copied_ptr<T>::copied_ptr(copied_ptr const & other)
{
	copy(other.get());
}


template <typename T>
copied_ptr<T> & copied_ptr<T>::operator=(copied_ptr const & other)
{
        if (&other != this) {
		delete ptr_;
		copy(other);
        }
        return *this;
}


template <typename T>
T & copied_ptr<T>::operator*() const
{
	return *ptr_;
}


template <typename T>
T * copied_ptr<T>::operator->() const
{
	return ptr_;
}


template <typename T>
T * copied_ptr<T>::get() const
{
	return ptr_;
}


template <typename T>
void copied_ptr<T>::copy(copied_ptr const & other)
{
	ptr_ = other.ptr_ ? new T(*other.ptr_) : 0;
}

} // namespace support
} // namespace lyx

#endif // NOT COPIED_PTR_H
