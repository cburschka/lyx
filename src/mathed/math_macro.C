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
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "debug.h"
#include "Painter.h"
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


const char * MathMacro::name() const
{
	return tmplate_->asMacroTemplate()->name().c_str();
}


bool MathMacro::defining() const
{
	return 0;
	//return mathcursor && mathcursor->formula()->getInsetName() == name();
}


void MathMacro::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, LM_TC_TEX, mi);
	mi_ = mi;

	if (defining()) {
		mathed_string_dim(font_, name(), ascent_, descent_, width_);
		return;
	}

	if (editing()) {
		expanded_ = tmplate_->xcell(0);
		expanded_.metrics(mi_);
		width_   = expanded_.width()   + 4;
		ascent_  = expanded_.ascent()  + 2;
		descent_ = expanded_.descent() + 2;

		width_ +=  mathed_string_width(font_, name()) + 10;

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(font_, "#1: ", lasc, ldes, lwid);

		for (idx_type i = 0; i < nargs(); ++i) {
			MathXArray const & c = xcell(i);
			c.metrics(mi_);
			width_    = max(width_, c.width() + lwid);
			descent_ += max(c.ascent(),  lasc) + 5;
			descent_ += max(c.descent(), ldes) + 5;
		}
		return;
	} 

	expanded_ = tmplate_->xcell(0);
	expanded_.data_.substitute(*this);
	expanded_.metrics(mi_);
	width_   = expanded_.width();
	ascent_  = expanded_.ascent();
	descent_ = expanded_.descent();
}


void MathMacro::draw(Painter & pain, int x, int y) const
{
	metrics(mi_);

	LyXFont texfont;
	whichFont(texfont, LM_TC_TEX, mi_);

	if (defining()) {
		drawStr(pain, texfont, x, y, name());
		return;
	}

	if (editing()) {
		int h = y - ascent() + 2 + expanded_.ascent();
		drawStr(pain, font_, x + 3, h, name());

		int const w = mathed_string_width(font_, name());
		expanded_.draw(pain, x + w + 12, h);
		h += expanded_.descent();

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(font_, "#1: ", lasc, ldes, lwid);

		for (idx_type i = 0; i < nargs(); ++i) {
			MathXArray const & c = xcell(i);
			h += max(c.ascent(), lasc) + 5;
			c.draw(pain, x + lwid, h);
			char str[] = "#1:";
			str[1] += static_cast<char>(i);
			drawStr(pain, texfont, x + 3, h, str);
			h += max(c.descent(), ldes) + 5;
		}
		return;
	}

	expanded_.draw(pain, x, y);
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


bool MathMacro::idxUp(idx_type & idx) const
{
	pos_type pos;
	return MathNestInset::idxLeft(idx, pos);
}


bool MathMacro::idxDown(idx_type & idx) const
{
	pos_type pos;
	return MathNestInset::idxRight(idx, pos);
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
	if (name() == "binom")
		features.require("binom");
	//MathInset::validate(features);
}


void MathMacro::maplize(MapleStream & os) const
{
	updateExpansion();
	::maplize(expanded_.data_, os);
}


void MathMacro::mathmlize(MathMLStream & os) const
{
	updateExpansion();
	::mathmlize(expanded_.data_, os);
}


void MathMacro::octavize(OctaveStream & os) const
{
	updateExpansion();
	::octavize(expanded_.data_, os);
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
	expanded_ = tmplate_->xcell(0);
	expanded_.data_.substitute(*this);
}
