// -*- C++ -*-
#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#include "math_inset.h"

/// things that need the dimension cache

class MathDimInset : public MathInset {
public:
	/// not sure whether the initialization is really necessary
	MathDimInset() : width_(0), ascent_(0), descent_(0) {}
	/// read ascent value (should be inline according to gprof) 
	int ascent() const { return ascent_; }
	/// read descent 
	int descent() const { return descent_; }
	/// read width
	int width() const { return width_; }

protected:
	///
	mutable int width_;
	///
	mutable int ascent_;
	///
	mutable int descent_;
};
#endif
