// -*- C++ -*-
#ifndef MATH_INFERINSET_H
#define MATH_INFERINSET_H

#include "math_gridinset.h"


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
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;

public:
	///
	MathArray opt_;
};

#endif
