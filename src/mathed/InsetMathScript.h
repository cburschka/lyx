// -*- C++ -*-
/**
 * \file InsetMathScript.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "InsetMathNest.h"


namespace lyx {


// An inset for super- and subscripts or both.  The 'nucleus' is always
// cell 0.  If there is just one script, it's cell 1 and cell_1_is_up_
// is set accordingly.  If both are used, cell 1 is up and cell 2 is down.

class InsetMathScript : public InsetMathNest {
public:
	/// create inset without scripts
	explicit InsetMathScript(Buffer * buf);
	/// create inset with single script
	explicit InsetMathScript(Buffer * buf, bool up);
	/// create inset with single script and given nucleus
	InsetMathScript(Buffer * buf, MathAtom const & at, bool up);
	///
	mode_type currentMode() const override { return MATH_MODE; }
	/// whether the inset has limit-like sub/superscript
	Limits limits() const override;
	/// sets types of sub/superscripts
	void limits(Limits lim) override;
	///
	MathClass mathClass() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter & pi, int x, int y) const override;

	/// move cursor backwards
	bool idxBackward(Cursor & cur) const override;
	/// move cursor forward
	bool idxForward(Cursor & cur) const override;
	/// move cursor up or down
	bool idxUpDown(Cursor & cur, bool up) const override;
	/// The index of the cell entered while moving backward
	size_type lastIdx() const override { return 0; }

	/// write LaTeX and Lyx code
	void write(TeXMathStream & os) const override;
	/// write normalized content
	void normalize(NormalStream &) const override;
	/// write content as something readable by Maple
	void maple(MapleStream &) const override;
	/// write content as something readable by Mathematica
	void mathematica(MathematicaStream &) const override;
	/// write content as MathML
	void mathmlize(MathMLStream &) const override;
	/// write content as HTML
	void htmlize(HtmlStream &) const override;
	/// write content as something readable by Octave
	void octave(OctaveStream &) const override;

	/// identifies scriptinsets
	InsetMathScript const * asScriptInset() const override;
	///
	InsetMathScript * asScriptInset() override;

	/// returns subscript. Always run 'hasDown' or 'has(false)' before!
	MathData const & down() const;
	/// returns subscript. Always run 'hasDown' or 'has(false)' before!
	MathData & down();
	/// returns superscript. Always run 'hasUp' or 'has(true)' before!
	MathData const & up() const;
	/// returns superscript. Always run 'hasUp' or 'has(true)' before!
	MathData & up();
	/// returns nucleus
	MathData const & nuc() const;
	/// returns nucleus
	MathData & nuc();
	/// do we have a superscript?
	bool hasUp() const;
	/// do we have a subscript?
	bool hasDown() const;
	/// do we have a script?
	bool has(bool up) const;
	/// what idx has super/subscript?
	idx_type idxOfScript(bool up) const;
	/// remove script
	void removeScript(bool up);
	/// make sure a script is accessible
	void ensure(bool up);
	/// say that we have scripts
	void infoize(odocstream & os) const override;
	/// say whether we have displayed limits
	void infoize2(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_SCRIPT_CODE; }
	///
	void validate(LaTeXFeatures &features) const override;
private:
	Inset * clone() const override;
	/// returns x offset for main part
	int dxx(BufferView const & bv) const;
	/// returns width of nucleus if any
	int nwid(BufferView const &) const;
	/// returns y offset for either superscript or subscript
	int dy01(BufferView const &, int asc, int des, int what) const;
	/// returns y offset for superscript
	int dy0(BufferView const &) const;
	/// returns y offset for subscript
	int dy1(BufferView const &) const;
	/// returns x offset for superscript
	int dx0(BufferView const & bv) const;
	/// returns x offset for subscript
	int dx1(BufferView const & bv) const;
	/// returns ascent of nucleus if any
	int nasc(BufferView const &) const;
	/// returns descent of nucleus if any
	int ndes(BufferView const &) const;
	/// Italic correction: amount of displacement between subscript and
	/// superscript in math mode as per Appendix G, rule 18f.  A positive value
	/// shifts the superscript to the right, and a negative value shifts the
	/// subscript to the left.
	int nker(BufferView const * bv) const;
	/// do we we have to draw the scripts above/below nucleus?
	bool hasLimits(FontInfo const &) const;
	/// clean up empty cells and return true if a cell has been deleted.
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;

	/// possible subscript (index 0) and superscript (index 1)
	bool cell_1_is_up_;
	/// remember whether we are in display mode (used by mathml output)
	mutable bool has_limits_ = false;
};


} // namespace lyx

#endif
