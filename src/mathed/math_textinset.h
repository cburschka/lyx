// -*- C++ -*-
/**
 * \file math_textinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_TEXTINSET_H
#define MATH_TEXTINSET_H

#include "math_gridinset.h"


// not yet a substitute for the real text inset...

class MathTextInset : public MathNestInset {
public:
	///
	MathTextInset();
	///
	std::auto_ptr<InsetBase> clone() const;
	/// get cursor position
	void getCursorPos(LCursor const & cur, int & x, int & y) const;
	/// this stores metrics information in cache_
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw according to cached metrics
	void draw(PainterInfo &, int x, int y) const;
	/// draw selection background
	//void drawSelection(PainterInfo & pi,
	//	idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const;
	/// moves cursor up or down
	//bool idxUpDown2(LCursor & pos, bool up) const;
protected:
	/// row corresponding to given position
	idx_type pos2row(pos_type pos) const;
	/// cached metrics
	mutable MathGridInset cache_;
};

#endif
