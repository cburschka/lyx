/**
 * \file InsetMathSplit.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSplit.h"

#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"

#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"

#include "support/lstrings.h"

#include <ostream>

using namespace std;

namespace lyx {

using support::bformat;


// FIXME: handle numbers in gui, currently they are only read and written

InsetMathSplit::InsetMathSplit(Buffer * buf, docstring const & name,
	char valign, bool numbered)
	: InsetMathGrid(buf, 1, 1, valign, docstring()), name_(name),
	  numbered_(numbered)
{
}


Inset * InsetMathSplit::clone() const
{
	return new InsetMathSplit(*this);
}


// FIXME: InsetMathGrid should be changed to let the real column alignment be
// given by a virtual method like displayColAlign, because the values produced
// by defaultColAlign can be invalidated by lfuns such as add-column. I suspect
// that for the moment the values produced by defaultColAlign are not used,
// notably because alignment is not implemented in the LyXHTML output.
char InsetMathSplit::defaultColAlign(col_type col)
{
	if (name_ == "gathered")
		return 'c';
	if (name_ == "lgathered")
		return 'l';
	if (name_ == "rgathered")
		return 'r';
	if (name_ == "split"
	    || name_ == "aligned"
	    || name_ == "align"
	    || name_ == "alignedat")
		return colAlign(hullAlign, col);
	return 'l';
}


char InsetMathSplit::displayColAlign(idx_type idx) const
{
	if (name_ == "gathered")
		return 'c';
	if (name_ == "lgathered")
		return 'l';
	if (name_ == "rgathered")
		return 'r';
	if (name_ == "split"
	    || name_ == "aligned"
	    || name_ == "align"
	    || name_ == "alignedat")
		return colAlign(hullAlign, col(idx));
	return InsetMathGrid::displayColAlign(idx);
}


int InsetMathSplit::displayColSpace(col_type col) const
{
	if (name_ == "split" || name_ == "aligned" || name_ == "align")
		return colSpace(hullAlign, col);
	if (name_ == "alignedat")
		return colSpace(hullAlignAt, col);
	return 0;
}



void InsetMathSplit::draw(PainterInfo & pi, int x, int y) const
{
	InsetMathGrid::draw(pi, x, y);
	setPosCache(pi, x, y);
}


bool InsetMathSplit::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_TABULAR_FEATURE: {
		string s = cmd.getArg(0);
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.message(bformat(
				from_utf8(N_("Can't add vertical grid lines in '%1$s'")),
				name_));
			flag.setEnabled(false);
			return true;
		}
		if (s == "align-left" || s == "align-center" || s == "align-right") {
			flag.setEnabled(false);
			return true;
		}
		break;
	}
	default:
		break;
	}
	return InsetMathGrid::getStatus(cur, cmd, flag);
}


void InsetMathSplit::write(WriteStream & ws) const
{
	MathEnsurer ensurer(ws);
	if (ws.fragile())
		ws << "\\protect";
	docstring suffix;
	if (!numbered_ && name_ == "align")
		suffix = from_ascii("*");
	ws << "\\begin{" << name_ << suffix << '}';
	bool open = ws.startOuterRow();
	if (name_ != "split" && name_ != "align" && verticalAlignment() != 'c')
		ws << '[' << verticalAlignment() << ']';
	if (name_ == "alignedat")
		ws << '{' << static_cast<unsigned int>((ncols() + 1)/2) << '}';
	InsetMathGrid::write(ws);
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\end{" << name_ << suffix << "}\n";
	if (open)
		ws.startOuterRow();
}


void InsetMathSplit::infoize(odocstream & os) const
{
	docstring name = name_;
	name[0] = support::uppercase(name[0]);
	if (name_ == "align" && !numbered_)
		os << name << "* ";
	else
		os << name << ' ';
}


void InsetMathSplit::mathmlize(MathStream & ms) const
{
	// split, gathered, aligned, alignedat
	// At the moment, those seem to display just fine without any
	// special treatment.
	// FIXME
	// lgathered and rgathered could use the proper alignment, but
	// it's not clear how to do that without copying a lot of code.
	// One idea would be to wrap the table in an <mrow>, and set the
	// alignment there via CSS.
	// FIXME how to handle numbered and unnumbered align?
	InsetMathGrid::mathmlize(ms);
}


void InsetMathSplit::htmlize(HtmlStream & ms) const
{
	// split, gathered, aligned, alignedat
	// At the moment, those seem to display just fine without any
	// special treatment.
	// FIXME
	// lgathered and rgathered could use the proper alignment.
	// FIXME how to handle numbered and unnumbered align?
	InsetMathGrid::htmlize(ms);
}


void InsetMathSplit::validate(LaTeXFeatures & features) const
{
	if (name_ == "split" || name_ == "gathered" || name_ == "aligned" ||
	    name_ == "alignedat" || name_ == "align")
		features.require("amsmath");
	else if (name_ == "lgathered" || name_ == "rgathered")
		features.require("mathtools");
	InsetMathGrid::validate(features);
}


} // namespace lyx
