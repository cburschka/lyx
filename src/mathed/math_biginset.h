// -*- C++ -*-
#ifndef MATH_BIGINSET_H
#define MATH_BIGINSET_H

#include "math_diminset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for \bigl & Co.
    \author André Pönitz
 */

class MathBigInset : public MathDimInset {
public:
	///
	MathBigInset(string const & name, string const & delim);
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
	size_type size() const;
	///
	double increase() const;
	
	/// \bigl or what?
	string const name_;
	/// ( or [ or Vert...
	string const delim_;
};

#endif
