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
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "support/std_ostream.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Undo.h"

#include "support/lstrings.h"


namespace lyx {

using support::bformat;

using std::endl;
using std::max;
using std::min;
using std::swap;
using std::auto_ptr;


InsetMathCases::InsetMathCases(row_type n)
	: InsetMathGrid(2, n, 'c', from_ascii("ll"))
{}


auto_ptr<Inset> InsetMathCases::doClone() const
{
	return auto_ptr<Inset>(new InsetMathCases(*this));
}


bool InsetMathCases::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim = dim_;
	InsetMathGrid::metrics(mi);
	dim_.wid += 8;

	if (dim_ == dim)
		return false;
	dim = dim_;
	return true;
}


void InsetMathCases::draw(PainterInfo & pi, int x, int y) const
{
	mathed_draw_deco(pi, x + 1, y - dim_.ascent(), 6, dim_.height(), from_ascii("{"));
	InsetMathGrid::drawWithMargin(pi, x, y, 8, 0);
	setPosCache(pi, x, y);
}


void InsetMathCases::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "*** InsetMathCases: request: " << cmd << endl;
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		recordUndo(cur);
		docstring const & s = cmd.argument();
		if (s == "add-vline-left" || s == "add-vline-right") {
			cur.undispatched();
			break;
		}
	}
	default:
		InsetMathGrid::doDispatch(cur, cmd);
	}
}


bool InsetMathCases::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		docstring const & s = cmd.argument();
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.enabled(false);
			flag.message(bformat(
				from_utf8(N_("No vertical grid lines in 'cases': feature %1$s")),
				s));
			return true;
		}
	}
	default:
		return InsetMathGrid::getStatus(cur, cmd, flag);
	}
}


void InsetMathCases::write(WriteStream & os) const
{
	if (os.fragile())
		os << "\\protect";
	os << "\\begin{cases}\n";
	InsetMathGrid::write(os);
	if (os.fragile())
		os << "\\protect";
	os << "\\end{cases}";
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


void InsetMathCases::infoize(odocstream & os) const
{
	os << "Cases ";
}


void InsetMathCases::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathGrid::validate(features);
}


} // namespace lyx
