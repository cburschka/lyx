#ifndef MATH_TEXTINSET_H
#define MATH_TEXTINSET_H

#include "math_gridinset.h"

// not yet a substitute for the real text inset...

class MathTextInset : public MathNestInset {
public:
	///
	MathTextInset();
	///
	MathInset * clone() const;
	/// get cursor position
	void getPos(idx_type idx, pos_type pos, int & x, int & y) const;
	/// this stores metrics information in cache_
	void metrics(MathMetricsInfo & mi) const;
	/// draw according to cached metrics
	void draw(MathPainterInfo &, int x, int y) const;
	/// draw selection background
	void drawSelection(MathPainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const;
	/// moves cursor up or down
	bool idxUpDown(idx_type &, pos_type & pos, bool up, int targetx) const;
protected:
	/// row corresponding to given position 
	idx_type pos2row(pos_type pos) const;
	/// cached metrics
	mutable MathGridInset cache_;
};

#endif
