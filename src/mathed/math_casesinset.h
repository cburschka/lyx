// -*- C++ -*-
#ifndef MATH_CASESINSET_H
#define MATH_CASESINSET_H

#include "math_gridinset.h"


class LaTeXFeatures;

class MathCasesInset : public MathGridInset {
public:
	///
	explicit MathCasesInset(row_type rows = 1u);
	///
	MathInset * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pain, int x, int y) const;

	///
	void infoize(std::ostream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void write(WriteStream & os) const;
	///
	void validate(LaTeXFeatures & features) const;
};

#endif
