// -*- C++ -*-
#ifndef MATH_FUNCLIMINSET_H
#define MATH_FUNCLIMINSET_H

#include "math_diminset.h"

// "normal" symbols that don't take limits and don't grow in displayed
// formulae

class MathFuncLimInset : public MathDimInset {
public:
	///
	explicit MathFuncLimInset(string const & name);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	bool isScriptable() const;

private:
	///
	string const name_;
	///
	mutable LyXFont font_;
};
#endif
