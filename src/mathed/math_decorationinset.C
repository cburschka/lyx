#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"


using std::ostream;


MathDecorationInset::MathDecorationInset(string const & name, int d)
	: MathInset(1, name), deco_(d)
{
	upper_ = deco_ != LM_underline && deco_ != LM_underbrace;
}


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}



void MathDecorationInset::Metrics(MathStyles st, int, int)
{
	xcell(0).Metrics(st);
	size_    = st;
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();

	dh_ = 5; //mathed_char_height(LM_TC_VAR, size(), 'I', ascent_, descent_);  

	if (upper_) {
		dy_ = -ascent_ - dh_;
		ascent_ += dh_ + 1;
	} else {
		dy_ = descent_ + 1;
		descent_ += dh_ + 2;
	}

	if (deco_ == LM_not) {
		ascent_  += dh_;
		descent_ += dh_;
		dh_ = height();
		dy_ = - ascent_;
	}
}

void MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(x);
	xcell(0).draw(pain, x, y);
	mathed_draw_deco(pain, x, y + dy_, width_, dh_, deco_);
}


void MathDecorationInset::Write(ostream & os, bool fragile) const
{
	if (fragile &&
			(name_ == "overbrace" ||
			 name_ == "underbrace" ||
			 name_ == "overleftarrow" ||
			 name_ == "overrightarrow"))
		os << "\\protect";
	os << '\\' << name_;

	if (deco_ == LM_not)
		os << ' ';
	else
		os << '{';

	cell(0).Write(os, fragile);  

	if (deco_ != LM_not)
		os << '}';
}

void MathDecorationInset::WriteNormal(ostream & os) const
{
	os << "[" << name_ << " ";
	cell(0).WriteNormal(os);
	os << "] ";
}
