// -*- C++ -*-
#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for super- and subscripts
    \author André Pönitz
 */

class MathScriptInset : public MathNestInset {
public:
	/// create inset without scripts
	MathScriptInset();
	/// create inset with single script
	explicit MathScriptInset(bool up);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & st) const;
	///
	void drawT(TextPainter &, int x, int y) const;

	///
	void metrics(MathInset const * nuc, MathMetricsInfo & st) const;
	///
	void draw(MathInset const * nuc, MathPainterInfo &, int x, int y) const;
	///
	void metricsT(MathInset const * nuc, TextMetricsInfo const & st) const;
	///
	void drawT(MathInset const * nuc, TextPainter &, int x, int y) const;
	///
	int ascent2(MathInset const * nuc) const;
	///
	int descent2(MathInset const * nuc) const;
	///
	int width2(MathInset const * nuc) const;

	///
	bool idxLeft(idx_type &, pos_type &) const;
	///
	bool idxRight(idx_type &, pos_type &) const;
	/// can we enter this cell?
	bool validCell(idx_type i) const { return script_[i]; }

	/// identifies scriptinsets
	MathScriptInset const * asScriptInset() const;
	///
	MathScriptInset * asScriptInset();

	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// get limits
	int limits() const { return limits_; }
	/// true if we have an "inner" position
	MathXArray const & up() const;
	/// returns subscript
	MathXArray const & down() const;
	/// returns superscript
	MathXArray & up();
	/// returns subscript
	MathXArray & down();
	/// do we have a superscript?
	bool hasUp() const;
	/// do we have a subscript?
	bool hasDown() const;
	/// do we have a script?
	bool has(bool up) const;
	/// do we have a anything?
	bool empty() const;
	/// remove script
	void removeScript(bool up);
	/// remove script
	void removeEmptyScripts();
	/// make sure a script is accessible
	void ensure(bool up);

	// call these methods ...2 to make compaq cxx in anal mode happy...
	/// suppresses empty braces if necessary
	virtual void write2(MathInset const * nuc, WriteStream & os) const;
	virtual void normalize2(MathInset const * nuc, NormalStream & os) const;
	virtual void octavize2(MathInset const * nuc, OctaveStream & os) const;
	virtual void maplize2(MathInset const * nuc, MapleStream & os) const;
	virtual void mathmlize2(MathInset const * nuc, MathMLStream & os) const;

public:
	/// returns x offset for main part
	int dxx(MathInset const * nuc) const;
	/// returns width of nucleus if any
	int nwid(MathInset const * nuc) const;
private:
	/// returns y offset for superscript
	int dy0(MathInset const * nuc) const;
	/// returns y offset for subscript
	int dy1(MathInset const * nuc) const;
	/// returns x offset for superscript
	int dx0(MathInset const * nuc) const;
	/// returns x offset for subscript
	int dx1(MathInset const * nuc) const;
	/// returns ascent of nucleus if any
	int nasc(MathInset const * nuc) const;
	/// returns descent of nucleus if any
	int ndes(MathInset const * nuc) const;
	/// where do we have to draw the scripts?
	bool hasLimits(MathInset const * nuc) const;

	/// possible subscript (index 0) and superscript (index 1)
	bool script_[2];
	/// 1 - "limits", -1 - "nolimits", 0 - "default"
	int limits_;
	/// cached font
	mutable LyXFont font_;
};

#endif
