#ifndef MATH_ERTINSET_H
#define MATH_ERTINSET_H

#include "math_textinset.h"

// implements support for \parbox

class MathErtInset : public MathTextInset {
public:
	///
	MathErtInset() {}
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
};

#endif
