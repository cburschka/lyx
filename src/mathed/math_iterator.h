// -*- C++ -*-
/**
 * \file math_iterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ITERATOR_H
#define MATH_ITERATOR_H

#include "cursor_slice.h"

#include <vector>


// this is used for traversing math insets

class MathIterator : private std::vector<CursorSlice> {
public:
	// re-use inherited stuff
	typedef std::vector<CursorSlice> base_type;
	using base_type::clear;
	using base_type::size;
	using base_type::push_back;
	using base_type::pop_back;
	using base_type::back;
	using base_type::begin;
	using base_type::end;
	using base_type::erase;
	using base_type::operator[];
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::const_iterator;
	friend bool operator!=(MathIterator const &, MathIterator const &);
	friend bool operator==(MathIterator const &, MathIterator const &);

	/// move on one step
	void operator++();
	/// read access to top most item
	MathArray const & cell() const;
};

///
bool operator==(MathIterator const &, MathIterator const &);
///
bool operator!=(MathIterator const &, MathIterator const &);

///
MathIterator ibegin(MathInset * p);
///
MathIterator iend(MathInset * p);

#endif
