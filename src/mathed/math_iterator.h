#ifndef MATH_ITERATOR_H
#define MATH_ITERATOR_H

#include "math_cursor.h"

// this helper struct is used for traversing math insets

class MathIterator {
public:
	/// default constructor, used for end of range
	MathIterator();
	/// start with given formula
	explicit MathIterator(MathInset * p);
	/// start with given position
	explicit MathIterator(MathCursor::cursor_type const & cursor);
	///
	MathCursorPos const & operator*() const;
	///
	MathCursorPos const & operator->() const;
	///
	void operator++();
	/// read access to top most item
	MathCursorPos const & position() const;
	/// write access to top most item
	MathCursorPos & position();
	/// read access to full path
	MathCursor::cursor_type const & cursor() const;
	/// read access to top most inset
	MathInset * par() const;
	
private:
	/// write access to top most item
	MathXArray const & xcell() const;
	/// write access to top most item
	MathInset * nextInset() const;
	/// own level down
	void push(MathInset *);
	/// own level up
	void pop();

	/// current position
	MathCursor::cursor_type cursor_;
};

///
bool operator==(MathIterator const &, MathIterator const &);
///
bool operator!=(MathIterator const &, MathIterator const &);


#endif
