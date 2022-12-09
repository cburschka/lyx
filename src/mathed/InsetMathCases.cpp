/**
 * \file InsetMathCases.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathCases.h"

#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "support/lstrings.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetMathCases::InsetMathCases(Buffer * buf, row_type rows)
	: InsetMathGrid(buf, 2, rows, 'c', from_ascii("ll"))
{}


Inset * InsetMathCases::clone() const
{
	return new InsetMathCases(*this);
}


void InsetMathCases::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeEnsureMath();
	InsetMathGrid::metrics(mi, dim);
}


void InsetMathCases::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	mathed_draw_deco(pi, x + 1, y - dim.ascent(), 6, dim.height(), from_ascii("{"));
	InsetMathGrid::draw(pi, x, y);
}


void InsetMathCases::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_TABULAR_FEATURE: {
		string s = cmd.getArg(0);
		// vertical lines and adding/deleting columns is not allowed for \cases
		if (s == "append-column" || s == "delete-column"
		    || s == "add-vline-left" || s == "add-vline-right") {
			cur.undispatched();
			return;
		}
	}
	default:
		break;
	}
	InsetMathGrid::doDispatch(cur, cmd);
}


bool InsetMathCases::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_TABULAR_FEATURE: {
		string s = cmd.getArg(0);
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.setEnabled(false);
			flag.message(bformat(
				from_utf8(N_("Can't add vertical grid lines in '%1$s'")),
				from_utf8("cases")));
			return true;
		}
		if (s == "append-column" || s == "delete-column") {
			flag.setEnabled(false);
			flag.message(bformat(
				from_utf8(N_("Changing number of columns not allowed in "
					     "'%1$s'")), from_utf8("cases")));
			return true;
		}
		break;
	}
	default:
		break;
	}
	return InsetMathGrid::getStatus(cur, cmd, flag);
}


void InsetMathCases::write(TeXMathStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile())
		os << "\\protect";
	bool open = os.startOuterRow();
	os << "\\begin{cases}\n";
	InsetMathGrid::write(os);
	if (os.fragile())
		os << "\\protect";
	os << "\\end{cases}";
	if (open)
		os.startOuterRow();
}


void InsetMathCases::normalize(NormalStream & os) const
{
	os << "[cases ";
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathCases::maple(MapleStream & os) const
{
	os << "cases(";
	InsetMathGrid::maple(os);
	os << ')';
}


void InsetMathCases::mathmlize(MathMLStream & ms) const
{
	ms << MTagInline("mo", "form='prefix' fence='true' stretchy='true' symmetric='true'")
	   << "{"
	   << ETagInline("mo");
	InsetMathGrid::mathmlize(ms);
}


// FIXME XHTML
// We need a brace here, somehow.
void InsetMathCases::htmlize(HtmlStream & ms) const
{
	InsetMathGrid::htmlize(ms, "class='cases'");
}


void InsetMathCases::infoize(odocstream & os) const
{
	os << "Cases ";
}


void InsetMathCases::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathGrid::validate(features);
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		// CSS based on eLyXer's, with modifications suggested in bug #8755
		features.addCSSSnippet(
			"table.cases{display: inline-block; text-align: center; border: none;"
			"border-left: thin solid black; vertical-align: middle; padding-left: 0.5ex;}\n"
			"table.cases td {text-align: left; border: none;}");
}


int InsetMathCases::displayColSpace(col_type) const
{
	return 20;
}


} // namespace lyx
