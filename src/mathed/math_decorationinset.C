#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathDecorationInset::MathDecorationInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}


bool MathDecorationInset::upper() const
{
	return
			name_ != "underbar" &&
			name_ != "underline" &&
			name_ != "underbrace";
}


bool MathDecorationInset::isScriptable() const
{
	return
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow";
}


bool MathDecorationInset::protect() const
{
	return
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow";
}


bool MathDecorationInset::wide() const
{
	return
			name_ == "overline" ||
			name_ == "underline" ||
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow" ||
			name_ == "widehat" ||
			name_ == "widetilde";
}


void MathDecorationInset::metrics(MathMetricsInfo const & st) const
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
	xcell(0).draw(pain, x, y);
	if (wide()) 
		mathed_draw_deco(pain, x, y + dy_, width_, dh_, name_);
	else {
		int w = 2 + mathed_char_width(LM_TC_VAR, size_, 'x'); 
		mathed_draw_deco(pain, x + (width_ - w) / 2, y + dy_, w, dh_, name_);
	}
}


void MathDecorationInset::write(WriteStream & os) const
{
	if (os.fragile() && protect())
		os << "\\protect";
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void MathDecorationInset::normalize(NormalStream & os) const
{
	os << "[deco " << name_ << ' ' <<  cell(0) << ']';
}
