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
