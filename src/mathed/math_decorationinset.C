#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/LOstream.h"


MathDecorationInset::MathDecorationInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{}


MathInset * MathDecorationInset::clone() const
{
	return new MathDecorationInset(*this);
}


bool MathDecorationInset::upper() const
{
	return key_->name.substr(0, 5) != "under";
}


bool MathDecorationInset::isScriptable() const
{
	return
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow" ||
			key_->name == "overleftrightarrow" ||
			key_->name == "underleftarrow" ||
			key_->name == "underrightarrow" ||
			key_->name == "underleftrightarrow";
}


bool MathDecorationInset::protect() const
{
	return
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow" ||
			key_->name == "overleftrightarrow" ||
			key_->name == "underleftarrow" ||
			key_->name == "underrightarrow" ||
			key_->name == "underleftrightarrow";
}


bool MathDecorationInset::wide() const
{
	return
			key_->name == "overline" ||
			key_->name == "underline" ||
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow" ||
			key_->name == "overleftrightarrow" ||
			key_->name == "widehat" ||
			key_->name == "widetilde" ||
			key_->name == "underleftarrow" ||
			key_->name == "underrightarrow" ||
			key_->name == "underleftrightarrow";
}


void MathDecorationInset::metrics(MathMetricsInfo & mi) const
{
	cell(0).metrics(mi);
	dim_ = cell(0).dim();
	dh_  = 6; //mathed_char_height(LM_TC_VAR, mi, 'I', ascent_, descent_);
	dw_  = 6; //mathed_char_width(LM_TC_VAR, mi, 'x');

	if (upper()) {
		dy_ = -dim_.a - dh_;
		dim_.a += dh_ + 1;
	} else {
		dy_ = dim_.d + 1;
		dim_.d += dh_ + 2;
	}

	metricsMarkers();
}


void MathDecorationInset::draw(MathPainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	if (wide())
		mathed_draw_deco(pi, x + 1, y + dy_, width(), dh_, key_->name);
	else
		mathed_draw_deco(pi, x + 1 + (width() - dw_) / 2, y + dy_, dw_, dh_, key_->name);
	drawMarkers(pi, x, y);
}


void MathDecorationInset::write(WriteStream & os) const
{
	if (os.fragile() && protect())
		os << "\\protect";
	os << '\\' << key_->name << '{' << cell(0) << '}';
}


void MathDecorationInset::normalize(NormalStream & os) const
{
	os << "[deco " << key_->name << ' ' <<  cell(0) << ']';
}


void MathDecorationInset::infoize(std::ostream & os) const
{
	os << "Deco: " << key_->name;
}
