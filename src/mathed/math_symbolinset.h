// -*- C++ -*-
/**
 * \file math_symbolinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SYMBOLINSET_H
#define MATH_SYMBOLINSET_H

#include "math_inset.h"

struct latexkeys;


/** "normal" symbols that don't take limits and don't grow in displayed
 *  formulae.
 */
class MathSymbolInset : public MathInset {
public:
	///
	explicit MathSymbolInset(latexkeys const * l);
	///
	explicit MathSymbolInset(char const * name);
	///
	explicit MathSymbolInset(string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	bool isRelOp() const;
	/// do we take scripts?
	bool isScriptable() const;
	/// do we take \limits or \nolimits?
	bool takesLimits() const;
	/// identifies SymbolInset as such
	MathSymbolInset const * asSymbolInset() const { return this; }
	/// the LaTeX name of the symbol (without the backslash)
	string name() const;
	///
	bool match(MathAtom const &) const;
	/// request "external features"
	void validate(LaTeXFeatures & features) const;

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
	///
	void infoize2(std::ostream & os) const;

private:
	///
	latexkeys const * sym_;
	///
	mutable int h_;
	///
	mutable bool scriptable_;
};
#endif
