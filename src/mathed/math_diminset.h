// -*- C++ -*-
#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#include "math_inset.h"
#include "dimension.h"


/// things that need the dimension cache

class MathDimInset : public MathInset {
public:
	/// not sure whether the initialization is really necessary
	MathDimInset() {}
	///
	Dimension dimensions() const { return dim_; }
	///
	void metricsT(TextMetricsInfo const &) const;
	///
	void drawT(TextPainter & pain, int x, int y) const;

protected:
	///
	mutable Dimension dim_;
};
#endif
