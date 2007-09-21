// -*- C++ -*-
/**
 * \file InsetMathPhantom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PHANTOMINSET_H
#define MATH_PHANTOMINSET_H

#include "InsetMathNest.h"


namespace lyx {

class InsetMathPhantom : public InsetMathNest {
public:
	///
	enum Kind {
		phantom,
		vphantom,
		hphantom
	};
	///
	explicit InsetMathPhantom(Kind);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
private:
	///
	virtual Inset * clone() const;
	///
	Kind kind_;
};



} // namespace lyx
#endif
