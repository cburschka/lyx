// -*- C++ -*-
#ifndef MATH_INFERINSET_H
#define MATH_INFERINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** for proof.sty's \infer
 *  \author André Poenitz
 *
 * Full author contact details are available in file CREDITS
 */
class MathInferInset : public MathGridInset {
public:
	///
	explicit MathInferInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;

public:
	///
	MathArray opt_;
};

#endif
