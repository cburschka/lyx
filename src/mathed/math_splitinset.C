#ifdef __GNUG__
#pragma implementation
#endif

#include "math_splitinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathSplitInset::MathSplitInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{
	setDefaults();
}


MathInset * MathSplitInset::clone() const
{
	return new MathSplitInset(*this);
}


char MathSplitInset::defaultColAlign(col_type col)
{
	if (name_ == "split")
		return 'l';
	if (name_ == "gathered")
		return 'c';
	if (name_ == "aligned")
		return (col & 1) ? 'l' : 'r';
	return 'l';
}


void MathSplitInset::write(WriteStream & ws) const
{
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\begin{" << name_ << "}";
	MathGridInset::write(ws);
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\end{" << name_ << "}\n";
}
