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
	///
	MathScriptInset();
	///
	explicit MathScriptInset(bool up);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	void metrics(MathInset const * nuc, MathMetricsInfo const & st) const;
	///
	void draw(MathInset const * nuc, Painter &, int x, int y) const;
	///
	int ascent(MathInset const * nuc) const;
	///
	int descent(MathInset const * nuc) const;
	///
	int width(MathInset const * nuc) const;

	///
	bool idxLeft(MathInset::idx_type &, MathInset::pos_type &) const;
	///
	bool idxRight(MathInset::idx_type &, MathInset::pos_type &) const;

	///
	MathScriptInset const * asScriptInset() const;
	///
	MathScriptInset * asScriptInset();

	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// 
	int limits() const { return limits_; }
	///
	bool hasLimits(MathInset const * nuc) const;
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
	/// remove script
	void removeScript(bool up);
	/// remove script
	void removeEmptyScripts();
	///
	void ensure(bool up);

	///
	void write(MathInset const * nuc, WriteStream & os) const;
	///
	void normalize(MathInset const * nuc, NormalStream & os) const;
	///
	void octavize(MathInset const * nuc, OctaveStream & os) const;
	///
	void maplize(MathInset const * nuc, MapleStream & os) const;
	///
	void mathmlize(MathInset const * nuc, MathMLStream & os) const;

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

	/// possible subscript (index 0) and superscript (index 1)
	bool script_[2]; 
	///
	int limits_;
	///
	mutable MathMetricsInfo mi_;
};

#endif

