// -*- C++ -*-
#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_inset.h"
#include "dimension.h"

/// things that need the dimension cache

class MathDimInset : public MathInset {
public:
	/// not sure whether the initialization is really necessary
	MathDimInset() {}
	/// read ascent value (should be inline according to gprof)
	int ascent() const { return dim_.ascent(); }
	/// read descent
	int descent() const { return dim_.descent(); }
	/// read width
	int width() const { return dim_.width(); }
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
