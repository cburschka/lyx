#include <config.h>

#include "math_exfuncinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

using std::auto_ptr;


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathExFuncInset::MathExFuncInset(string const & name, MathArray const & ar)
	: MathNestInset(1), name_(name)
{
	cell(0) = ar;
}


auto_ptr<InsetBase> MathExFuncInset::clone() const
{
	return auto_ptr<InsetBase>(new MathExFuncInset(*this));
}


void MathExFuncInset::metrics(MetricsInfo & mi, Dimension & /*dim*/) const
{
	mathed_string_dim(mi.base.font, name_, dim_);
}


void MathExFuncInset::draw(PainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
}


string MathExFuncInset::name() const
{
	return name_;
}


void MathExFuncInset::maple(MapleStream & os) const
{
	if (name_ == "det")
		os << "linalg[det](" << cell(0) << ')';
	else
		os << name_ << '(' << cell(0) << ')';
}


void MathExFuncInset::maxima(MaximaStream & os) const
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


void MathExFuncInset::mathematica(MathematicaStream & os) const
{
	os << asMathematicaName(name_) << '[' << cell(0) << ']';
}


void MathExFuncInset::mathmlize(MathMLStream & os) const
{
	os << MTag(name_.c_str()) << cell(0) << ETag(name_.c_str());
}


void MathExFuncInset::octave(OctaveStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}
