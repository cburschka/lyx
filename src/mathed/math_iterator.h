// -*- C++ -*-
#ifndef MATH_ITERATOR_H
#define MATH_ITERATOR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_pos.h"
#include <vector>

// this is used for traversing math insets

class MathIterator : private std::vector<MathCursorPos> {
public:
	// re-use inherited stuff
	typedef std::vector<MathCursorPos> base_type;
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

	/// default constructor
	MathIterator();
	/// start with given inset
	explicit MathIterator(MathInset * p);
	///
	MathCursorPos const & operator*() const;
	///
	MathCursorPos const & operator->() const;
	/// move on one step
	void operator++();
	/// move on several steps
	void jump(difference_type);
	/// read access to top most inset
	MathInset const * par() const;
	/// read access to top most inset
	MathInset * par();
	/// helper for iend
	void goEnd();
	/// read access to top most item
	MathArray const & cell() const;
	/// shrinks to at most i levels
	void shrink(size_type i);

private:
	/// own level down
	void push(MathInset *);
	/// own level up
	void pop();
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
