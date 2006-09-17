/**
 * \file InsetMathAMSArray.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFeatures.h"
#include "InsetMathAMSArray.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"


using lyx::docstring;

using std::string;
using std::auto_ptr;
using lyx::support::bformat;


InsetMathAMSArray::InsetMathAMSArray(string const & name, int m, int n)
	: InsetMathGrid(m, n), name_(name)
{}


InsetMathAMSArray::InsetMathAMSArray(string const & name)
	: InsetMathGrid(1, 1), name_(name)
{}


auto_ptr<InsetBase> InsetMathAMSArray::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathAMSArray(*this));
}


char const * InsetMathAMSArray::name_left() const
{
	if (name_ == "bmatrix")
		return "[";
	if (name_ == "Bmatrix")
		return "{";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	if (name_ == "pmatrix")
		return "(";
	return ".";
}


char const * InsetMathAMSArray::name_right() const
{
	if (name_ == "bmatrix")
		return "]";
	if (name_ == "Bmatrix")
		return "}";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	if (name_ == "pmatrix")
		return ")";
	return ".";
}


void InsetMathAMSArray::metrics(MetricsInfo & mi, Dimension & dim) const
{
	ArrayChanger dummy(mi.base);
	InsetMathGrid::metrics(mi, dim);
	dim.wid += 14;
	dim_ = dim;
}


void InsetMathAMSArray::draw(PainterInfo & pi, int x, int y) const
{
	int const yy = y - dim_.ascent();
	// Drawing the deco after an ArrayChanger does not work
	mathed_draw_deco(pi, x + 1, yy, 5, dim_.height(), name_left());
	mathed_draw_deco(pi, x + dim_.width() - 8, yy, 5, dim_.height(), name_right());
	ArrayChanger dummy(pi.base);
	InsetMathGrid::drawWithMargin(pi, x, y, 6, 8);
}


bool InsetMathAMSArray::getStatus(LCursor & cur, FuncRequest const & cmd,
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


void InsetMathAMSArray::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}';
	InsetMathGrid::write(os);
	os << "\\end{" << name_ << '}';
}


void InsetMathAMSArray::infoize(std::ostream & os) const
{
	string name = name_;
	name[0] = lyx::support::uppercase(name[0]);
	os << name << ' ';
}


void InsetMathAMSArray::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathAMSArray::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathGrid::validate(features);
}
