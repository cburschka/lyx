/**
 * \file InsetMathKern.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathKern.h"
#include "MathMLStream.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "dimension.h"


namespace lyx {

using std::string;
using std::auto_ptr;


InsetMathKern::InsetMathKern()
{}


InsetMathKern::InsetMathKern(LyXLength const & w)
	: wid_(w)
{}


InsetMathKern::InsetMathKern(string const & s)
	: wid_(s)
{}


auto_ptr<InsetBase> InsetMathKern::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathKern(*this));
}


void InsetMathKern::metrics(MetricsInfo & mi, Dimension & dim) const
{
	wid_pix_ = wid_.inPixels(0, mathed_char_width(mi.base.font, 'M'));
	dim.wid = wid_pix_;
	dim.asc = 0;
	dim.des = 0;
}


int InsetMathKern::width() const
{
	return wid_pix_;
}


void InsetMathKern::draw(PainterInfo &, int, int) const
{}


void InsetMathKern::write(WriteStream & os) const
{
	os << "\\kern" << wid_.asLatexString() << ' ';
}


void InsetMathKern::normalize(NormalStream & os) const
{
	os << "[kern " << wid_.asLatexString() << ']';
}


} // namespace lyx
