/**
 * \file math_amsarrayinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFeatures.h"
#include "math_amsarrayinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"

#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"


using std::string;
using std::auto_ptr;
using lyx::support::bformat;


MathAMSArrayInset::MathAMSArrayInset(string const & name, int m, int n)
	: MathGridInset(m, n), name_(name)
{}


MathAMSArrayInset::MathAMSArrayInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{}


auto_ptr<InsetBase> MathAMSArrayInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathAMSArrayInset(*this));
}


char const * MathAMSArrayInset::name_left() const
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


char const * MathAMSArrayInset::name_right() const
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


void MathAMSArrayInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	MetricsInfo m = mi;
	if (m.base.style == LM_ST_DISPLAY)
		m.base.style = LM_ST_TEXT;
	MathGridInset::metrics(m, dim);
	dim.wid += 12;
	dim_ = dim;
}


void MathAMSArrayInset::draw(PainterInfo & pi, int x, int y) const
{
	MathGridInset::drawWithMargin(pi, x, y, 6, 6);
	int const yy = y - dim_.ascent();
	mathed_draw_deco(pi, x + 1, yy, 5, dim_.height(), name_left());
	mathed_draw_deco(pi, x + dim_.width() - 6, yy, 5, dim_.height(), name_right());
	setPosCache(pi, x, y);
}


bool MathAMSArrayInset::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		string const s = cmd.argument;
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.message(bformat(
			N_("Can't add vertical grid lines in '%1$s'"),
				name_));
			flag.enabled(false);
			return true;
		}
		return MathGridInset::getStatus(cur, cmd, flag);
	}
	default:
		return MathGridInset::getStatus(cur, cmd, flag);
        }
}


void MathAMSArrayInset::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}';
	MathGridInset::write(os);
	os << "\\end{" << name_ << '}';
}


void MathAMSArrayInset::infoize(std::ostream & os) const
{
	string name = name_;
	name[0] = lyx::support::uppercase(name[0]);
	os << name << ' ';
}


void MathAMSArrayInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	MathGridInset::normalize(os);
	os << ']';
}


void MathAMSArrayInset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	MathGridInset::validate(features);
}
