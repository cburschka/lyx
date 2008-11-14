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
	InsetMathScript();
	/// create inset with single script
	explicit InsetMathScript(bool up);
	/// create inset with single script and given nucleus
	InsetMathScript(MathAtom const & at, bool up);
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
	/// write content as something readable by Maple
	void maple(MapleStream &) const;
	/// write content as something readable by Mathematica
	void mathematica(MathematicaStream &) const;
	/// write content as something resembling MathML
	void mathmlize(MathStream &) const;
	/// write content as something readable by Octave
	void octave(OctaveStream &) const;

	/// identifies scriptinsets
	InsetMathScript const * asScriptInset() const;
	///
	InsetMathScript * asScriptInset();

	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// get limits
	int limits() const { return limits_; }
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
	void infoize(odocstream & os) const;
	/// say whether we have displayed limits
	void infoize2(odocstream & os) const;
protected:
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	virtual Inset * clone() const;
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
	/// returns superscript kerning of nucleus if any
	int nker(BufferView const * bv) const;
	/// where do we have to draw the scripts?
	bool hasLimits() const;
	/// clean up empty cells and return true if a cell has been deleted.
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	/// possible subscript (index 0) and superscript (index 1)
	bool cell_1_is_up_;
	/// 1 - "limits", -1 - "nolimits", 0 - "default"
	int limits_;
};



} // namespace lyx
#endif
