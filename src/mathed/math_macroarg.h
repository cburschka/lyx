// -*- C++ -*-
#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "math_parinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A macro argument
    \author Alejandro Aguilar Sierra
*/
class MathMacroArgument : public MathParInset {
public:
	///
	explicit
	MathMacroArgument(int);
	///
	MathedInset * Clone();
	///
	void Metrics();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	/// Is expanded or not
	void setExpand(bool e);
	/// Is expanded or not
	bool getExpand() const;
private:
	///
	bool expnd_mode_;
	///
	int number_;
};
#endif
