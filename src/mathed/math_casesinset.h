// -*- C++ -*-
/**
 * \file math_casesinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CASESINSET_H
#define MATH_CASESINSET_H

#include "math_gridinset.h"


class LaTeXFeatures;

class MathCasesInset : public MathGridInset {
public:
	///
	explicit MathCasesInset(row_type rows = 1u);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

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
