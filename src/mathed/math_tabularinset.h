// -*- C++ -*-
/**
 * \file math_tabularinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_TABULARINSET_H
#define MATH_TABULARINSET_H

#include "math_gridinset.h"


/// Inset for things like \begin{tabular}...\end{tabular}
class MathTabularInset : public MathGridInset {
public:
	///
	MathTabularInset(std::string const &, int m, int n);
	///
	MathTabularInset(std::string const &, int m, int n,
		char valign, std::string const & halign);
	///
	MathTabularInset(std::string const &, char valign, std::string const & halign);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	MathTabularInset * asTabularInset() { return this; }
	///
	MathTabularInset const * asTabularInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;

private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	std::string name_;
};

#endif
