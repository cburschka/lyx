// -*- C++ -*-
#ifndef MATH_CASESINSET_H
#define MATH_CASESINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathCasesInset : public MathGridInset {
public:
	///
	explicit MathCasesInset(row_type rows = 1u);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter & pain, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void write(WriteStream & os) const;
};

#endif
