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


/// An inset for super- and subscripts.
class MathScriptInset : public MathNestInset {
public:
	/// create inset without scripts
	MathScriptInset();
	/// create inset with single script
	explicit MathScriptInset(bool up);
	/// create inset with single script and given nucleus
	MathScriptInset(MathAtom const & at, bool up);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;

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
	/// move cursor left
	bool idxLeft(idx_type &, pos_type &) const;
	/// move cursor right
	bool idxRight(idx_type &, pos_type &) const;
	/// move cursor up or down
	bool idxUpDown(idx_type & idx, pos_type & pos, bool up, int targetx) const;
	/// Target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(idx_type & idx, pos_type & pos) const;
	/// can we enter this cell?
	bool validCell(idx_type i) const { return i == 2 || script_[i]; }

	/// identifies scriptinsets
	MathScriptInset const * asScriptInset() const;
	///
	MathScriptInset * asScriptInset();

	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// get limits
	int limits() const { return limits_; }
	/// returns subscript
	MathArray const & down() const;
	/// returns subscript
	MathArray & down();
	/// returns superscript
	MathArray const & up() const;
	/// returns superscript
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
	/// remove script
	void removeScript(bool up);
	/// make sure a script is accessible
	void ensure(bool up);
	/// say that we have scripts
	void infoize(std::ostream & os) const;
	/// say whether we have displayed limits
	void infoize2(std::ostream & os) const;
	/// local dispatcher
	dispatch_result dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);

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
	void notifyCursorLeaves(idx_type idx);

	/// possible subscript (index 0) and superscript (index 1)
	bool script_[2];
	/// 1 - "limits", -1 - "nolimits", 0 - "default"
	int limits_;
};

#endif
