#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sizeinset.h"
#include "support/LOstream.h"


MathSizeInset::MathSizeInset(MathStyles st)
	: MathInset(1), style_(st)
{
	name_ = verbose();
}


char const * MathSizeInset::verbose() const
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


void MathSizeInset::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);
	xcell(0).draw(pain, x, y);
}


void MathSizeInset::Metrics(MathStyles /* st */, int, int)
{
	xcell(0).Metrics(style_);
	ascent_   = xcell(0).ascent_;
	descent_  = xcell(0).descent_;
	width_    = xcell(0).width_;
}


void MathSizeInset::Write(std::ostream & os, bool fragile) const
{
	os << "{\\" << name() << " ";
	cell(0).Write(os, fragile);
	os << "}";
}


void MathSizeInset::WriteNormal(std::ostream & os) const
{
	os << "[" << name() << " ";
	cell(0).WriteNormal(os);
	os << "]";
}
