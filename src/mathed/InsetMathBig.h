// -*- C++ -*-
/**
 * \file InsetMathBig.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BIGINSET_H
#define MATH_BIGINSET_H

#include "InsetMath.h"

#include <string>


namespace lyx {

/// Inset for \\bigl & Co.
class InsetMathBig : public InsetMath {
public:
	///
	InsetMathBig(docstring const & name, docstring const & delim);
	///
	docstring name() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void infoize2(odocstream & os) const;
	///
	static bool isBigInsetDelim(docstring const &);

private:
	virtual Inset * clone() const;
	///
	size_type size() const;
	///
	double increase() const;

	/// \\bigl or what?
	docstring const name_;
	/// ( or [ or \\Vert...
	docstring const delim_;
};


} // namespace lyx

#endif
