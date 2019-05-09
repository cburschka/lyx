/**
 * \file InsetMathExFunc.cpp
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
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "support/docstream.h"

using namespace std;

namespace lyx {


InsetMathExFunc::InsetMathExFunc(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 1), name_(name)
{}


InsetMathExFunc::InsetMathExFunc(Buffer * buf, docstring const & name, MathData const & ar)
	: InsetMathNest(buf, 1), name_(name)
{
	cell(0) = ar;
}


Inset * InsetMathExFunc::clone() const
{
	return new InsetMathExFunc(*this);
}


void InsetMathExFunc::metrics(MetricsInfo & mi, Dimension & dim) const
{
	metricsStrRedBlack(mi, dim, name_);
}


void InsetMathExFunc::draw(PainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
}


docstring InsetMathExFunc::name() const
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


static string asMathematicaName(string const & name)
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


static docstring asMathematicaName(docstring const & name)
{
	return from_utf8(asMathematicaName(to_utf8(name)));
}


void InsetMathExFunc::mathematica(MathematicaStream & os) const
{
	os << asMathematicaName(name_) << '[' << cell(0) << ']';
}


void InsetMathExFunc::mathmlize(MathStream & ms) const
{
	ms << "<" << from_ascii(ms.namespacedTag("mi")) << ">"
	   << name_
       << "</" << from_ascii(ms.namespacedTag("mi")) << ">"
	   << "<" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << "&af;"
	   << "</" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << cell(0);
}


void InsetMathExFunc::htmlize(HtmlStream & os) const
{
	os << name_ << cell(0);
}


void InsetMathExFunc::octave(OctaveStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}


} // namespace lyx
