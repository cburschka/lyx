// -*- C++ -*-
/**
 * \file math_arrayinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "math_gridinset.h"


/// Inset for things like \begin{array}...\end{array}
class MathArrayInset : public MathGridInset {
public:
	///
	MathArrayInset(std::string const &, int m, int n);
	///
	MathArrayInset(std::string const &, int m, int n,
		char valign, std::string const & halign);
	///
	MathArrayInset(std::string const &, char valign, std::string const & halign);
	/// convenience constructor from whitespace/newline separated data
	MathArrayInset(std::string const &, std::string const & str);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	MathArrayInset * asArrayInset() { return this; }
	///
	MathArrayInset const * asArrayInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void maple(MapleStream & os) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	std::string name_;
};

#endif
