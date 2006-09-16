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
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"


using lyx::docstring;
using lyx::support::bformat;
using std::string;
using std::auto_ptr;


InsetMathSplit::InsetMathSplit(string const & name)
	: InsetMathGrid(1, 1), name_(name)
{
	setDefaults();
}


auto_ptr<InsetBase> InsetMathSplit::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathSplit(*this));
}


char InsetMathSplit::defaultColAlign(col_type col)
{
	if (name_ == "split")
		return 'l';
	if (name_ == "gathered")
		return 'c';
	if (name_ == "aligned")
		return (col & 1) ? 'l' : 'r';
	if (name_ == "alignedat")
		return (col & 1) ? 'l' : 'r';
	return 'l';
}


void InsetMathSplit::draw(PainterInfo & pi, int x, int y) const
{
	InsetMathGrid::draw(pi, x, y);
	setPosCache(pi, x, y);
}


bool InsetMathSplit::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		docstring const & s = cmd.argument();
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.message(bformat(
			lyx::from_utf8(N_("Can't add vertical grid lines in '%1$s'")),
			lyx::from_utf8(name_)));
			flag.enabled(false);
			return true;
		}
		return InsetMathGrid::getStatus(cur, cmd, flag);
	}
	default:
		return InsetMathGrid::getStatus(cur, cmd, flag);
	}
}


void InsetMathSplit::write(WriteStream & ws) const
{
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\begin{" << name_ << '}';
	if (name_ == "alignedat")
		ws << '{' << static_cast<unsigned int>((ncols() + 1)/2) << '}';
	InsetMathGrid::write(ws);
	if (ws.fragile())
		ws << "\\protect";
	ws << "\\end{" << name_ << "}\n";
}


void InsetMathSplit::infoize(std::ostream & os) const
{
	string name = name_;
	name[0] = lyx::support::uppercase(name[0]);
	os << name << ' ';
}


void InsetMathSplit::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}
