// -*- C++ -*-
#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#include "math_inset.h"

/// thing that need the dimension cache
class MathDimInset : public MathInset {
public:
	MathDimInset();
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

protected:
	///
	int width_;
	///
	int ascent_;
	///
	int descent_;
};
#endif
