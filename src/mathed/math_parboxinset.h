#ifndef MATH_PARBOXINSET_H
#define MATH_PARBOXINSET_H

#include "math_nestinset.h"

class MathParboxInset : public MathNestInset {
public:
	///
	MathParboxInset();
	///
	MathParboxInset * asParboxInset() { return this; }
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	void setWidth(string const & width);
	///
	void setPosition(string const & pos);
private:
	///
	void rebreak();
	/// width on screen
	int lyx_width_;
	/// width for TeX
	string tex_width_;
	///
	char position_;
};

#endif
