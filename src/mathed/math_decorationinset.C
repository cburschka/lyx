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
	return name_.substr(0, 5) != "under";
}


bool MathDecorationInset::isScriptable() const
{
	return
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow" ||
			name_ == "overleftrightarrow" ||
			name_ == "underleftarrow" ||
			name_ == "underrightarrow" ||
			name_ == "underleftrightarrow";
}


bool MathDecorationInset::protect() const
{
	return
			name_ == "overbrace" ||
			name_ == "underbrace" ||
			name_ == "overleftarrow" ||
			name_ == "overrightarrow" ||
			name_ == "overleftrightarrow" ||
			name_ == "underleftarrow" ||
			name_ == "underrightarrow" ||
			name_ == "underleftrightarrow";
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
			name_ == "overleftrightarrow" ||
			name_ == "widehat" ||
			name_ == "widetilde" ||
			name_ == "underleftarrow" ||
			name_ == "underrightarrow" ||
			name_ == "underleftrightarrow";
}


void MathDecorationInset::metrics(MathMetricsInfo const & mi) const
{
	xcell(0).metrics(mi);
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();

	dh_ = 6; //mathed_char_height(LM_TC_VAR, mi(), 'I', ascent_, descent_);
	dw_ = 6; //mathed_char_width(LM_TC_VAR, mi, 'x');

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
	else
		mathed_draw_deco(pain, x + (width_ - dw_) / 2, y + dy_, dw_, dh_, name_);
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
