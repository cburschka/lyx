// -*- C++ -*-
#ifndef MATH_TABULARINSET_H
#define MATH_TABULARINSET_H

#include "math_gridinset.h"


/**
 * Inset for things like \begin{tabular}...\end{tabular}
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class MathTabularInset : public MathGridInset {
public:
	///
	MathTabularInset(string const &, int m, int n);
	///
	MathTabularInset(string const &, int m, int n,
		char valign, string const & halign);
	///
	MathTabularInset(string const &, char valign, string const & halign);
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	MathTabularInset * asTabularInset() { return this; }
	///
	MathTabularInset const * asTabularInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;

private:
	///
	string name_;
};

#endif
