/**
 * \file InsetMathExFunc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathExFunc.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "MathStream.h"
#include "MathSupport.h"


using std::string;
using std::auto_ptr;


InsetMathExFunc::InsetMathExFunc(string const & name)
	: InsetMathNest(1), name_(name)
{}


InsetMathExFunc::InsetMathExFunc(string const & name, MathArray const & ar)
	: InsetMathNest(1), name_(name)
{
	cell(0) = ar;
}


auto_ptr<InsetBase> InsetMathExFunc::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathExFunc(*this));
}


void InsetMathExFunc::metrics(MetricsInfo & mi, Dimension & /*dim*/) const
{
	mathed_string_dim(mi.base.font, name_, dim_);
}


void InsetMathExFunc::draw(PainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
}


string InsetMathExFunc::name() const
{
	return name_;
}


void InsetMathExFunc::maple(MapleStream & os) const
{
	if (name_ == "det")
		os << "linalg[det](" << cell(0) << ')';
	else
		os << name_ << '(' << cell(0) << ')';
}


void InsetMathExFunc::maxima(MaximaStream & os) const
{
	if (name_ == "det")
		os << "determinant(" << cell(0) << ')';
	else
		os << name_ << '(' << cell(0) << ')';
}


string asMathematicaName(string const & name)
{
	if (name == "sin")    return "Sin";
	if (name == "sinh")   return "Sinh";
	if (name == "arcsin") return "ArcSin";
	if (name == "asin")   return "ArcSin";
	if (name == "cos")    return "Cos";
	if (name == "cosh")   return "Cosh";
	if (name == "arccos") return "ArcCos";
	if (name == "acos")   return "ArcCos";
	if (name == "tan")    return "Tan";
	if (name == "tanh")   return "Tanh";
	if (name == "arctan") return "ArcTan";
	if (name == "atan")   return "ArcTan";
	if (name == "cot")    return "Cot";
	if (name == "coth")   return "Coth";
	if (name == "csc")    return "Csc";
	if (name == "sec")    return "Sec";
	if (name == "exp")    return "Exp";
	if (name == "log")    return "Log";
	if (name == "ln" )    return "Log";
	if (name == "arg" )   return "Arg";
	if (name == "det" )   return "Det";
	if (name == "gcd" )   return "GCD";
	if (name == "max" )   return "Max";
	if (name == "min" )   return "Min";
	if (name == "erf" )   return "Erf";
	if (name == "erfc" )  return "Erfc";
	return name;
}


void InsetMathExFunc::mathematica(MathematicaStream & os) const
{
	os << asMathematicaName(name_) << '[' << cell(0) << ']';
}


void InsetMathExFunc::mathmlize(MathMLStream & os) const
{
	os << MTag(name_.c_str()) << cell(0) << ETag(name_.c_str());
}


void InsetMathExFunc::octave(OctaveStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}
