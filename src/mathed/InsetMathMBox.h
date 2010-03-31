// -*- C++ -*-
/**
 * \file InsetMathMBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#define MATH_MBOX

#ifdef MATH_MBOX

#ifndef MATH_MBOXINSET_H
#define MATH_MBOXINSET_H

#include "InsetMath.h"

#include "insets/InsetText.h"


namespace lyx {

class Buffer;
class BufferView;

// almost a substitute for the real text inset...

class InsetMathMBox : public InsetMath {
public:
	///
	explicit InsetMathMBox(Buffer * buffer);
	explicit InsetMathMBox(Buffer * buffer, Layout const & layout);

	/// this stores metrics information in cache_
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw according to cached metrics
	void draw(PainterInfo &, int x, int y) const;
	///
	bool inMathed() const { return false; }
	///
	bool isActive() const { return true; }

	///
	void write(WriteStream & os) const;
	///
	int latex(odocstream & os, OutputParams const & runparams) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	Text * getText(int) const;
	///
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	InsetCode lyxCode() const { return MATH_MBOX_CODE; }

protected:
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);

	///
	mutable InsetText text_;

private:
	virtual Inset * clone() const;
};



} // namespace lyx

#endif //MATH_MBOXINSET_H

#endif //MATH_MBOX
