// -*- C++ -*-
#ifndef MATH_ENVINSET_H
#define MATH_ENVINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


/** Environtments á la \begin{something}...\end{something}
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
*/

class MathEnvInset : public MathNestInset {
public:
	///
	MathEnvInset(string const & name_);
	///
	MathInset * clone() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void metrics(MetricsInfo & mi) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// name of that environment
	string name_;
};

#endif
