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

#include "math_nestinset.h"
#include "math_data.h"


/// This class contains the data for a macro.
class MathMacro : public MathDimInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(std::string const &);
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	MathMacro * asMacro() { return this; }
	///
	MathMacro const * asMacro() const { return this; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawExpanded(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void metricsExpanded(MetricsInfo & mi, Dimension & dim) const;
	///
	int widthExpanded() const;
	///
	std::string name() const;
	///
	void setExpansion(MathArray const & exp, MathArray const & args) const;

	///
	void validate(LaTeXFeatures &) const;

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
	void updateExpansion() const;

	///
	std::string name_;
	///
	mutable MathArray expanded_;
	///
	mutable MathArray args_;
};


#endif
