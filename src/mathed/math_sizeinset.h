// -*- C++ -*-
#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


/** An inset for \scriptsize etc
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
*/

class latexkeys;

class MathSizeInset : public MathNestInset {
public:
	///
	explicit MathSizeInset(latexkeys const * l);
	///
	MathInset * clone() const;
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	latexkeys const * key_;
	///
	Styles const style_;
};

#endif
