// -*- C++ -*-
/**
 * \file InsetMathSideset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SIDESETINSET_H
#define MATH_SIDESETINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// An inset for amsmath \sideset. The 'nucleus' is always cell 0.
/// cell(1) is the bottom left index, cell(2) is the top left index,
/// cell(3) is the bottom right index, and cell(4) is top right index.
class InsetMathSideset : public InsetMathNest {
public:
	///
	InsetMathSideset(Buffer * buf, bool scriptl, bool scriptr);
	/// create inset with given nucleus
	InsetMathSideset(Buffer * buf, bool scriptl, bool scriptr,
	                 MathAtom const & at);
	///
	mode_type currentMode() const { return MATH_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;

	/// move cursor backwards
	bool idxBackward(Cursor & cur) const;
	/// move cursor forward
	bool idxForward(Cursor & cur) const;
	/// move cursor up or down
	bool idxUpDown(Cursor & cur, bool up) const;
	/// Target pos when we enter the inset while moving forward
	bool idxFirst(Cursor & cur) const;
	/// Target pos when we enter the inset while moving backwards
	bool idxLast(Cursor & cur) const;

	/// write LaTeX and Lyx code
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream &) const;
	/// write content as MathML
	void mathmlize(MathStream &) const;
	/// write content as HTML
	void htmlize(HtmlStream &) const;

	/// returns nucleus
	MathData const & nuc() const { return cell(0); };
	/// returns nucleus
	MathData & nuc()             { return cell(0); };
	/// bottom left index or single left cell
	MathData const & bl() const  { return cell(1); }
	/// bottom left index or single left cell
	MathData & bl()              { return cell(1); }
	/// top left index or single left cell
	MathData const & tl() const  { return cell(1 + scriptl_); }
	/// top left index or single left cell
	MathData & tl()              { return cell(1 + scriptl_); }
	/// bottom right index or single right cell
	MathData const & br() const  { return cell(2 + scriptl_); }
	/// bottom right index or single right cell
	MathData & br()              { return cell(2 + scriptl_); }
	/// top right index or single right cell
	MathData const & tr() const  { return cell(2 + scriptl_ + scriptr_); }
	/// top right index or single right cell
	MathData & tr()              { return cell(2 + scriptl_ + scriptr_); }
	/// say that we have scripts
	void infoize(odocstream & os) const;
	///
	InsetCode lyxCode() const { return MATH_SCRIPT_CODE; }
	///
	void validate(LaTeXFeatures &features) const;
private:
	virtual Inset * clone() const;
	/// returns x offset of nucleus
	int dxn(BufferView const & bv) const;
	/// returns width of nucleus if any
	int nwid(BufferView const &) const;
	/// returns y offset for either superscript or subscript
	int dybt(BufferView const &, int asc, int des, bool top) const;
	/// returns y offset for superscript
	int dyt(BufferView const &) const;
	/// returns y offset for subscript
	int dyb(BufferView const &) const;
	/// returns x offset for right subscript and superscript
	int dxr(BufferView const & bv) const;
	/// returns ascent of nucleus if any
	int nasc(BufferView const &) const;
	/// returns descent of nucleus if any
	int ndes(BufferView const &) const;
	/// returns subscript and superscript kerning of nucleus if any
	int nker(BufferView const * bv) const;
	/// Whether there are two left scripts or one single cell
	bool scriptl_; 
	/// Whether there are two right scripts or one single cell
	bool scriptr_; 
};


} // namespace lyx

#endif
