#ifndef MATH_POS_H
#define MATH_POS_H

#include <iosfwd>
#include "math_xdata.h"


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
	/// returns xcell corresponding to this position
	MathXArray & xcell() const;
	/// returns xcell corresponding to this position
	MathXArray & xcell(MathArray::idx_type idx) const;
	///
	int xpos() const;
	///
	int ypos() const;

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
