/**
 * \file math_splitinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_splitinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

using std::auto_ptr;


MathSplitInset::MathSplitInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{
	setDefaults();
}


auto_ptr<InsetBase> MathSplitInset::clone() const
{
	return auto_ptr<InsetBase>(new MathSplitInset(*this));
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
	ws << "\\begin{" << name_ << '}';
	MathGridInset::write(ws);
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\end{" << name_ << "}\n";
}
