#ifndef MATH_POS_H
#define MATH_POS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>
#include "math_data.h"


/// Description of a position
class MathCursorPos {
public:
	///
	MathCursorPos();
	///
	explicit MathCursorPos(MathInset *);

	/// returns cell corresponding to this position
	MathArray & cell() const;
	/// returns cell corresponding to this position
	MathArray & cell(MathArray::idx_type idx) const;
	/// gets screen position of the thing
	void getPos(int & x, int & y) const;
	/// set position
	void setPos(MathArray::pos_type pos);

public:
	/// pointer to an inset
	MathInset * par_;
	/// cell index of a position in this inset
	MathArray::idx_type idx_;
	/// position in this cell
	MathArray::pos_type pos_;
};

/// test for equality
bool operator==(MathCursorPos const &, MathCursorPos const &);
/// test for inequality
bool operator!=(MathCursorPos const &, MathCursorPos const &);
/// test for order
bool operator<(MathCursorPos const &, MathCursorPos const &);
/// output
std::ostream & operator<<(std::ostream &, MathCursorPos const &);

#endif
