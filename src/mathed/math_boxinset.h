// -*- C++ -*-
#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "math_diminset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class InsetText;
class BufferView;
class Buffer;
class LyXFont;

/// Support for \\mbox

class MathBoxInset : public MathDimInset {
public:
	///
	explicit MathBoxInset(string const &);
	///
	MathBoxInset(MathBoxInset const &);
	///
	~MathBoxInset();
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const &) const;
	/// identifies BoxInsets
	MathBoxInset * asBoxInset() { return this; }
	///
	bool isHyperActive() const { return 1; }

private:
	/// unimplemented
	void operator=(MathBoxInset const &);

	///
	mutable MathMetricsInfo mi_;
	///
	string name_;
	///
	InsetText * text_;
	///
	mutable Buffer * buffer_;	
};
#endif
