#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"


using std::ostream;


MathDecorationInset::MathDecorationInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}


bool MathDecorationInset::upper() const
{
	return name_ != "underline" && name_ != "underbrace";
}


bool MathDecorationInset::protect() const
{
	return
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow";
}


void MathDecorationInset::metrics(MathStyles st) const
{
	xcell(0).metrics(st);
	size_    = st;
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();

	dh_ = 5; //mathed_char_height(LM_TC_VAR, size(), 'I', ascent_, descent_);  

	if (upper()) {
		dy_ = -ascent_ - dh_;
		ascent_ += dh_ + 1;
	} else {
		dy_ = descent_ + 1;
		descent_ += dh_ + 2;
	}
}


void MathDecorationInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(x);
	xcell(0).draw(pain, x, y);
	mathed_draw_deco(pain, x, y + dy_, width_, dh_, name_);
}


void MathDecorationInset::write(ostream & os, bool fragile) const
{
	if (fragile && protect())
		os << "\\protect";
	os << '\\' << name_ << '{';
	cell(0).write(os, fragile);  
	os << '}';
}


void MathDecorationInset::writeNormal(ostream & os) const
{
	os << "[" << name_ << " ";
	cell(0).writeNormal(os);
	os << "] ";
}
