/**
 * \file math_substackinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFeatures.h"
#include "math_substackinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "support/std_ostream.h"

#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"

#include "support/lstrings.h"


using lyx::support::bformat;
using std::string;
using std::auto_ptr;


MathSubstackInset::MathSubstackInset()
	: MathGridInset(1, 1)
{}


auto_ptr<InsetBase> MathSubstackInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathSubstackInset(*this));
}


void MathSubstackInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY) {
		StyleChanger dummy(mi.base, LM_ST_TEXT);
		MathGridInset::metrics(mi, dim);
	} else {
		MathGridInset::metrics(mi, dim);
	}
	dim_ = dim;
}


void MathSubstackInset::draw(PainterInfo & pi, int x, int y) const
{
	MathGridInset::draw(pi, x + 1, y);
}


bool MathSubstackInset::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		string const name("substack");
		string const s = cmd.argument;
		if (s == "add-vline-left" || s == "add-vline-right") {
			flag.message(bformat(
			N_("Can't add vertical grid lines in '%1$s'"), name));
			flag.enabled(false);
			return true;
		}
		return MathGridInset::getStatus(cur, cmd, flag);
	}
	default:
		return MathGridInset::getStatus(cur, cmd, flag);
	}
}


void MathSubstackInset::infoize(std::ostream & os) const
{
	os << "Substack ";
}


void MathSubstackInset::write(WriteStream & os) const
{
	os << "\\substack{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathSubstackInset::normalize(NormalStream & os) const
{
	os << "[substack ";
	MathGridInset::normalize(os);
	os << ']';
}


void MathSubstackInset::maple(MapleStream & os) const
{
	os << "substack(";
	MathGridInset::maple(os);
	os << ')';
}


void MathSubstackInset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	MathGridInset::validate(features);
}
