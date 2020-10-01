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


namespace lyx {


/// Environtments á la \begin{something}...\end{something}
class InsetMathEnv : public InsetMathNest {
public:
	///
	InsetMathEnv(Buffer * buf, docstring const & name_);
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_ENV_CODE; }

private:
	Inset * clone() const override;
	/// name of that environment
	docstring name_;
};


} // namespace lyx

#endif
