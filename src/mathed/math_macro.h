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
#include "math_nestinset.h"
#include "math_macrotable.h"


/// This class contains the data for a macro.
class MathMacro : public MathNestInset {
public:
	/// A macro can be built from an existing template
	MathMacro(std::string const & name, int numargs);
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawExpanded(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
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
	void expand() const;

	/// name of macro
	std::string name_;
	/// the unexpanded macro defintition
	mutable MathArray tmpl_;
	/// the matcro substituted with our args
	mutable MathArray expanded_;
};


#endif
