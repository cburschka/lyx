// -*- C++ -*-
/**
 * \file math_scriptinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_nestinset.h"


// An inset for super- and subscripts or both.  The 'nucleus' is always
// cell 0.  If there is just one script, it's cell 1 and cell_1_is_up_
// is set accordingly.  If both are used, cell 1 is up and cell 2 is down.

class MathScriptInset : public MathNestInset {
public:
	/// create inset without scripts
	MathScriptInset();
	/// create inset with single script
	explicit MathScriptInset(bool up);
	/// create inset with single script and given nucleus
	MathScriptInset(MathAtom const & at, bool up);
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;

	/// move cursor left
	bool idxLeft(LCursor & cur) const;
	/// move cursor right
	bool idxRight(LCursor & cur) const;
	/// move cursor up or down
	bool idxUpDown(LCursor & cur, bool up) const;
	/// Target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(LCursor & cur) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(LCursor & cur) const;

	/// write LaTeX and Lyx code
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream &) const;
	/// write content as something readable by Maple
	void maple(MapleStream &) const;
	/// write content as something readable by Mathematica
	void mathematica(MathematicaStream &) const;
	/// write content as something resembling MathML
	void mathmlize(MathMLStream &) const;
	/// write content as something readable by Octave
	void octave(OctaveStream &) const;

	/// identifies scriptinsets
	MathScriptInset const * asScriptInset() const;
	///
	MathScriptInset * asScriptInset();

	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// get limits
	int limits() const { return limits_; }
	/// returns subscript. Always run 'hasDown' or 'has(false)' before!
	MathArray const & down() const;
	/// returns subscript. Always run 'hasDown' or 'has(false)' before!
	MathArray & down();
	/// returns superscript. Always run 'hasUp' or 'has(true)' before!
	MathArray const & up() const;
	/// returns superscript. Always run 'hasUp' or 'has(true)' before!
	MathArray & up();
	/// returns nucleus
	MathArray const & nuc() const;
	/// returns nucleus
	MathArray & nuc();
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
	void infoize(std::ostream & os) const;
	/// say whether we have displayed limits
	void infoize2(std::ostream & os) const;
protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
private:
	/// returns x offset for main part
	int dxx() const;
	/// returns width of nucleus if any
	int nwid() const;
	/// returns y offset for superscript
	int dy0() const;
	/// returns y offset for subscript
	int dy1() const;
	/// returns x offset for superscript
	int dx0() const;
	/// returns x offset for subscript
	int dx1() const;
	/// returns ascent of nucleus if any
	int nasc() const;
	/// returns descent of nucleus if any
	int ndes() const;
	/// where do we have to draw the scripts?
	bool hasLimits() const;
	/// clean up empty cells
	void notifyCursorLeaves(LCursor & cur);

	/// possible subscript (index 0) and superscript (index 1)
	bool cell_1_is_up_;
	/// 1 - "limits", -1 - "nolimits", 0 - "default"
	int limits_;
};

#endif
