// -*- C++ -*-
#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#include "math_inset.h"

/// things that need the dimension cache

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
	mutable int width_;
	///
	mutable int ascent_;
	///
	mutable int descent_;
};
#endif
