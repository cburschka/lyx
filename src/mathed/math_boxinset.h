// -*- C++ -*-
#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXFont;

/// Support for \\mbox

class MathBoxInset : public MathNestInset {
public:
	///
	explicit MathBoxInset(string const & name);
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	string name_;
};


#endif
