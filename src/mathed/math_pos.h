#ifndef MATH_POS_H
#define MATH_POS_H


#include <iosfwd>
#include "math_data.h"


/// Description of a position
class CursorPos {
public:
	///
	CursorPos();
	///
	explicit CursorPos(MathInset *);

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
	MathInset * inset_;
	/// cell index of a position in this inset
	MathArray::idx_type idx_;
	/// position in this cell
	MathArray::pos_type pos_;
};

/// test for equality
bool operator==(CursorPos const &, CursorPos const &);
/// test for inequality
bool operator!=(CursorPos const &, CursorPos const &);
/// test for order
bool operator<(CursorPos const &, CursorPos const &);
/// output
std::ostream & operator<<(std::ostream &, CursorPos const &);

#endif
