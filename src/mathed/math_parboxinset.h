// -*- C++ -*-
#ifndef MATH_PARBOXINSET_H
#define MATH_PARBOXINSET_H


#include "math_textinset.h"

// implements support for \parbox

class MathParboxInset : public MathTextInset {
public:
	///
	MathParboxInset();
	///
	MathParboxInset * asParboxInset() { return this; }
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	void setWidth(string const & width);
	///
	void setPosition(string const & pos);
private:
	/// width on screen
	int lyx_width_;
	/// width for TeX
	string tex_width_;
	/// one of htb
	char position_;
};

#endif
