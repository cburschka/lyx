// -*- C++ -*-
/**
 * \file math_charinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CHARINSET_H
#define MATH_CHARINSET_H

#include "math_inset.h"


/// The base character inset.
class MathCharInset : public MathInset {
public:
	///
	explicit MathCharInset(char c);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void octave(OctaveStream & os) const;
	/// identifies Charinsets
	MathCharInset const * asCharInset() const { return this; }
	///
	char getChar() const { return char_; }
	///
	bool isRelOp() const;
	///
	bool match(MathInset const *) const;

private:
	/// the character
	char char_;
};
#endif
