/*
 *  File:        math_macro.C
 *  Purpose:     Implementation of macro class for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Math
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.2, Math & Lyx project.
 *
 *  This code is under the GNU General Public Licence version 2 or later.
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macro.h"
#include "math_support.h"
#include "math_extern.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LaTeXFeatures.h"


using std::max;


MathMacro::MathMacro(string const & name)
	: MathNestInset(MathMacroTable::provide(name)->asMacroTemplate()->numargs()),
		tmplate_(MathMacroTable::provide(name))
{}


MathMacro::MathMacro(MathMacro const & m)
	: MathNestInset(m),
		tmplate_(m.tmplate_) // don't copy 'expanded_'!
{}



MathInset * MathMacro::clone() const
{
	return new MathMacro(*this);
}


string MathMacro::name() const
{
	return tmplate_->asMacroTemplate()->name();
}


bool MathMacro::defining() const
{
	return 0;
	//return mathcursor && mathcursor->formula()->getInsetName() == name();
}


void MathMacro::expand() const
{
	expanded_ = tmplate_->xcell(tmplate_->cell(1).empty() ? 0 : 1);
}


void MathMacro::metrics(MathMetricsInfo & mi) const
{
	augmentFont(font_, "lyxtex");
	mi_ = mi;

	if (defining()) {
		mathed_string_dim(font_, name(), dim_);
		return;
	}

	if (editing()) {
		expand();
		dim_ = expanded_.metrics(mi_);
		metricsMarkers2(2); 

		dim_.w +=  mathed_string_width(font_, name()) + 10;

		Dimension ldim;
		mathed_string_dim(font_, "#1: ", ldim);

		for (idx_type i = 0; i < nargs(); ++i) {
			MathXArray const & c = xcell(i);
			c.metrics(mi_);
			dim_.w  = max(dim_.w, c.width() + ldim.w);
			dim_.d += max(c.ascent(),  ldim.a) + 5;
			dim_.d += max(c.descent(), ldim.d) + 5;
		}
		return;
	}

	expand();
	expanded_.data().substitute(*this);
	expanded_.metrics(mi_);
	dim_ = expanded_.dim();
}


void MathMacro::draw(MathPainterInfo & pi, int x, int y) const
{
	metrics(mi_);

	LyXFont texfont;
	augmentFont(texfont, "lyxtex");

	if (defining()) {
		drawStr(pi, texfont, x, y, name());
		return;
	}

	if (editing()) {
		int h = y - ascent() + 2 + expanded_.ascent();
		drawStr(pi, font_, x + 3, h, name());

		int const w = mathed_string_width(font_, name());
		expanded_.draw(pi, x + w + 12, h);
		h += expanded_.descent();

		Dimension ldim;
		mathed_string_dim(font_, "#1: ", ldim);

		for (idx_type i = 0; i < nargs(); ++i) {
			MathXArray const & c = xcell(i);
			h += max(c.ascent(), ldim.a) + 5;
			c.draw(pi, x + ldim.w, h);
			char str[] = "#1:";
			str[1] += static_cast<char>(i);
			drawStr(pi, texfont, x + 3, h, str);
			h += max(c.descent(), ldim.d) + 5;
		}
		return;
	}

	expanded_.draw(pi, x, y);
}


void MathMacro::dump() const
{
	MathMacroTable::dump();
	lyxerr << "\n macro: '" << this << "'\n";
	lyxerr << " name: '" << name() << "'\n";
	lyxerr << " template: '";
	WriteStream wi(lyxerr);
	tmplate_->write(wi);
	lyxerr << "'\n";
}


bool MathMacro::idxUpDown(idx_type & idx, pos_type &, bool up, int x) const
{
	pos_type pos;
	if (up) {
		if (!MathNestInset::idxLeft(idx, pos))
			return false;
		pos = xcell(idx).x2pos(x);
		return true;
	} else {
		if (!MathNestInset::idxRight(idx, pos))
			return false;
		pos = xcell(idx).x2pos(x);
		return true;
	}
}


bool MathMacro::idxLeft(idx_type &, pos_type &) const
{
	return false;
}


bool MathMacro::idxRight(idx_type &, pos_type &) const
{
	return false;
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (name() == "binom" || name() == "mathcircumflex")
		features.require(name());
	//MathInset::validate(features);
}


void MathMacro::maplize(MapleStream & os) const
{
	updateExpansion();
	::maplize(expanded_.data(), os);
}


void MathMacro::mathmlize(MathMLStream & os) const
{
	updateExpansion();
	::mathmlize(expanded_.data(), os);
}


void MathMacro::octavize(OctaveStream & os) const
{
	updateExpansion();
	::octavize(expanded_.data(), os);
}


void MathMacro::normalize(NormalStream & os) const
{
	os << "[macro " << name() << " ";
	for (idx_type i = 0; i < nargs(); ++i)
		os << cell(i) << ' ';
	os << ']';
}


void MathMacro::write(WriteStream & os) const
{
	os << '\\' << name();
	for (idx_type i = 0; i < nargs(); ++i)
		os << '{' << cell(i) << '}';
	if (nargs() == 0)
		os << ' ';
}


void MathMacro::updateExpansion() const
{
	expand();
	expanded_.data().substitute(*this);
}
