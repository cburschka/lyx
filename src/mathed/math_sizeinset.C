#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sizeinset.h"
#include "support/LOstream.h"


MathSizeInset::MathSizeInset(MathStyles st)
	: MathNestInset(1), style_(st)
{}


char const * MathSizeInset::name() const
{
	switch (style_) {
		case LM_ST_DISPLAY:
			return "displaystyle";
		case LM_ST_TEXT:
			return "textstyle";
		case LM_ST_SCRIPT:
			return "scriptstyle";
		case LM_ST_SCRIPTSCRIPT:
			return "scriptscriptstyle";
	}
	return "unknownstyle";
}


MathInset * MathSizeInset::clone() const
{
	return new MathSizeInset(*this);
}


void MathSizeInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	xcell(0).draw(pain, x, y);
}


void MathSizeInset::metrics(MathStyles /* st */) const
{
	xcell(0).metrics(style_);
	ascent_   = xcell(0).ascent_;
	descent_  = xcell(0).descent_;
	width_    = xcell(0).width_;
}


void MathSizeInset::write(std::ostream & os, bool fragile) const
{
	os << "{\\" << name() << " ";
	cell(0).write(os, fragile);
	os << "}";
}


void MathSizeInset::writeNormal(std::ostream & os) const
{
	os << "[" << name() << " ";
	cell(0).writeNormal(os);
	os << "]";
}
