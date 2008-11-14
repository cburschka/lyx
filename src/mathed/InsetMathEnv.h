// -*- C++ -*-
/**
 * \file InsetMathEnv.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ENVINSET_H
#define MATH_ENVINSET_H

#include "InsetMathNest.h"

#include <string>


namespace lyx {


/// Environtments á la \begin{something}...\end{something}
class InsetMathEnv : public InsetMathNest {
public:
	///
	InsetMathEnv(docstring const & name_);
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void infoize(odocstream & os) const;

private:
	virtual Inset * clone() const;
	/// name of that environment
	docstring name_;
};


} // namespace lyx

#endif
