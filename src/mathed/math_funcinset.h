// -*- C++ -*-
#ifndef MATH_FUNCINSET_H
#define MATH_FUNCINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 Functions or LaTeX names for objects that I don't know how to draw.
 */
class MathFuncInset : public MathInset {
public:
	///
	explicit MathFuncInset(string const & nm, short ot = LM_OT_FUNC);
	///
	MathInset *  Clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
	///
	bool GetLimits() const;
private:
	///
	bool lims_;
};
#endif
