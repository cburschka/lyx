// -*- C++ -*-
/**
 * \file unique_ptr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_UNIQUE_PTR_H
#define LYX_UNIQUE_PTR_H

#include <memory>

namespace lyx { using std::unique_ptr; }


/// Define lyx::make_unique() across platforms

#ifdef HAVE_DEF_MAKE_UNIQUE

namespace lyx { using std::make_unique; }

#else
// For all other compilers:
// using https://isocpp.org/files/papers/N3656.txt

#include <cstddef>
#include <type_traits>
#include <utility>


namespace lyx {

namespace {

template<class T> struct _Unique_if {
	typedef unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]> {
	typedef unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N> struct _Unique_if<T[N]> {
	typedef void _Known_bound;
};

} //anon namespace

template<class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args&&... args) {
	return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n) {
	typedef typename std::remove_extent<T>::type U;
	return unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args&&...) = delete;

} // namespace lyx

#endif // definition of make_unique

#endif // LYX_UNIQUE_PTR_H
