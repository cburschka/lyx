// -*- C++ -*-
#ifndef MATH_XARROWINSET_H
#define MATH_XARROWINSET_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Wide arrows like \xrightarrow
    \author André Pönitz
 */

class MathXArrowInset : public MathNestInset {
public:
	///
	explicit MathXArrowInset(string const & name);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void normalize(NormalStream & os) const;

private:
	///
	bool upper() const;
	///
	string const name_;
};
#endif
