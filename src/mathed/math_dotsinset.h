// -*- C++ -*-
/**
 * \file math_dotsinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_diminset.h"


class latexkeys;

/// The different kinds of ellipsis
class MathDotsInset : public MathDimInset {
public:
	///
	explicit MathDotsInset(latexkeys const * l);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	string name() const;
protected:
	/// cache for the thing's height
	mutable int dh_;
	///
	latexkeys const * key_;
};
#endif
