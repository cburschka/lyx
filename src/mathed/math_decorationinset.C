#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"


using std::ostream;


MathDecorationInset::MathDecorationInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{
}


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}


bool MathDecorationInset::upper() const
{
	return key_->id != LM_underline && key_->id != LM_underbrace;
}


bool MathDecorationInset::protect() const
{
	return
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow";
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

	if (key_->id == LM_not) {
		ascent_  += dh_;
		descent_ += dh_;
		dh_ = height();
		dy_ = - ascent_;
	}
}

void MathDecorationInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(x);
	xcell(0).draw(pain, x, y);
	mathed_draw_deco(pain, x, y + dy_, width_, dh_, key_);
}


void MathDecorationInset::write(ostream & os, bool fragile) const
{
	string name = key_->name;
	if (fragile && protect())
		os << "\\protect";
	os << '\\' << name;

	if (key_->id == LM_not)
		os << ' ';
	else
		os << '{';

	cell(0).write(os, fragile);  

	if (key_->id != LM_not)
		os << '}';
}


void MathDecorationInset::writeNormal(ostream & os) const
{
	os << "[" << key_->name << " ";
	cell(0).writeNormal(os);
	os << "] ";
}
