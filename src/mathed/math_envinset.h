// -*- C++ -*-
#ifndef MATH_ENVINSET_H
#define MATH_ENVINSET_H

#include "math_nestinset.h"
#include "math_metricsinfo.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Environtments á la \begin{something}...\end{something}
    \author André Pönitz
*/

class MathEnvInset : public MathNestInset {
public:
	///
	MathEnvInset(string const & name_);
	///
	MathInset * clone() const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// name of that environment
	string name_;
};

#endif
