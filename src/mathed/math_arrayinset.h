// -*- C++ -*-
#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "math_gridinset.h"


/**
 * Inset for things like \begin{array}...\end{array}
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class MathArrayInset : public MathGridInset {
public:
	///
	MathArrayInset(string const &, int m, int n);
	///
	MathArrayInset(string const &, int m, int n,
		char valign, string const & halign);
	///
	MathArrayInset(string const &, char valign, string const & halign);
	/// convienience constructor from whitespace/newline seperated data
	MathArrayInset(string const &, string const & str);
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	MathArrayInset * asArrayInset() { return this; }
	///
	MathArrayInset const * asArrayInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void maple(MapleStream & os) const;

private:
	///
	string name_;
};

#endif
