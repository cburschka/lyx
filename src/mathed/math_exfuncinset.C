
#ifdef __GNUG__
#pragma implementation 
#endif

#include <config.h>

#include "math_exfuncinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathExFuncInset::MathExFuncInset(string const & name, MathArray const & ar)
	: MathNestInset(1), name_(name)
{
	cell(0) = ar;
}


MathInset * MathExFuncInset::clone() const
{
	return new MathExFuncInset(*this);
}


void MathExFuncInset::metrics(MathMetricsInfo & mi) const
{
	mathed_string_dim(mi.base.font, name_, dim_);
}


void MathExFuncInset::draw(MathPainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
}


string MathExFuncInset::name() const
{
	return name_;
}


void MathExFuncInset::maplize(MapleStream & os) const
{
	if (name_ == "det")
		os << "linalg[det](" << cell(0) << ')';
	else
		os << name_ << '(' << cell(0) << ')';
}


string asMathematicaName(string const & name)
{
	if (name == "sin")    return "Sin";
	if (name == "sinh")   return "Sinh";
	if (name == "arcsin") return "ArcSin";
	if (name == "cos")    return "Cos";
	if (name == "cosh")   return "Cosh";
	if (name == "arcos")  return "ArcCos";
	if (name == "tan")    return "Tan";
	if (name == "tanh")   return "Tanh";
	if (name == "arctan") return "ArcTan";
	if (name == "cot")    return "Cot";
	if (name == "coth")   return "Coth";
	if (name == "csc")    return "Csc";
	if (name == "sec")    return "Sec";
	if (name == "exp")    return "Exp";
	if (name == "log")    return "Log";
	if (name == "ln" )    return "Log";
	return name;
}


void MathExFuncInset::mathematicize(MathematicaStream & os) const
{
	os << asMathematicaName(name_) << '[' << cell(0) << ']';
}


void MathExFuncInset::mathmlize(MathMLStream & os) const
{
	os << MTag(name_.c_str()) << cell(0) << ETag(name_.c_str());
}


void MathExFuncInset::octavize(OctaveStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}
