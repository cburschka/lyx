#ifdef __GNUG__
#pragma implementation
#endif

#include "math_splitinset.h"
#include "math_mathmlstream.h"


MathSplitInset::MathSplitInset(int n)
	: MathGridInset(2, n)
{
	setDefaults();
}


MathInset * MathSplitInset::clone() const
{
	return new MathSplitInset(*this);
}


void MathSplitInset::write(WriteStream & ws) const
{
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\begin{split}";
	MathGridInset::write(ws);
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\end{split}\n";
}
