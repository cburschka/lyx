/**
 * \file math_numberinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_numberinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


using std::string;
using std::auto_ptr;


MathNumberInset::MathNumberInset(string const & s)
	: str_(s)
{}


auto_ptr<InsetBase> MathNumberInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathNumberInset(*this));
}


void MathNumberInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
}


void MathNumberInset::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pi, pi.base.font, x, y, str_);
}


void MathNumberInset::normalize(NormalStream & os) const
{
	os << "[number " << str_ << ']';
}


void MathNumberInset::maple(MapleStream & os) const
{
	os << str_;
}


void MathNumberInset::octave(OctaveStream & os) const
{
	os << str_;
}


void MathNumberInset::mathmlize(MathMLStream & os) const
{
	os << "<mi> " << str_ << " </mi>";
}


void MathNumberInset::write(WriteStream & os) const
{
	os << str_;
}
