// -*- C++ -*-
/**
 * \file math_macro.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACRO_H
#define MATH_MACRO_H

#include "math_data.h"
#include "math_nestinset.h"
#include "metricsinfo.h"
#include "support/std_string.h"

class MathMacroTemplate;


/// This class contains the data for a macro.
class MathMacro : public MathNestInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(string const &);
	///
	MathMacro(MathMacro const &);
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void dump() const;

	///
	bool idxUpDown(idx_type & idx, pos_type & pos, bool up, int targetx) const;
	///
	bool idxLeft(idx_type & idx, pos_type & pos) const;
	///
	bool idxRight(idx_type & idx, pos_type & pos) const;

	///
	void validate(LaTeXFeatures &) const;
	///
	bool isMacro() const { return true; }
	///
	bool match(MathAtom const &) const { return false; }

	///
	void maple(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void infoize(std::ostream &) const;
	///
	void infoize2(std::ostream &) const;

private:
	///
	void operator=(MathMacro const &);
	///
	string name() const;
	///
	bool defining() const;
	///
	void updateExpansion() const;
	///
	void expand() const;

	///
	MathAtom & tmplate_;
	///
	mutable MathArray expanded_;
	///
	mutable MetricsInfo mi_;
	///
	mutable LyXFont font_;
};


#endif
