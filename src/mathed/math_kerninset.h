// -*- C++ -*-
#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "math_inset.h"
#include "vspace.h"
#include "LString.h"
#include "math_nestinset.h"


/// The \kern primitive
/// Some hack for visual effects

class MathKernInset : public MathInset {
public:
	///
	MathKernInset();
	///
	explicit MathKernInset(LyXLength const & wid);
	///
	explicit MathKernInset(string const & wid);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
private:
	/// width in em
	LyXLength wid_;
};
#endif
