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

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macro.h"
#include "array.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "debug.h"
#include "mathed/support.h"
#include "mathed/math_cursor.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "Painter.h"

using std::endl;

MathMacro::MathMacro(MathMacroTemplate const & t)
	: MathInset(t.numargs(), t.name(), LM_OT_MACRO), tmplate_(&t)
{}


MathInset * MathMacro::clone() const
{
	return new MathMacro(*this);
}


void MathMacro::Metrics(MathStyles st, int, int)
{
	if (mathcursor && mathcursor->isInside(this)) {
		expanded_ = tmplate_->xcell(0);
		expanded_.Metrics(st);
		size_    = st;
		width_   = expanded_.width()   + 4;
		ascent_  = expanded_.ascent()  + 2;
		descent_ = expanded_.descent() + 2;

		width_ +=  mathed_string_width(LM_TC_TEXTRM, size_, name_) + 10;

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(LM_TC_TEXTRM, size_, "#1: ", lasc, ldes, lwid);

		for (int i = 0; i < nargs(); ++i) {
			MathXArray & c = xcell(i);
			c.Metrics(st);
			width_    = std::max(width_, c.width() + lwid);
			descent_ += std::max(c.ascent(),  lasc) + 5;
			descent_ += std::max(c.descent(), ldes) + 5;
		}
	} else {
		expanded_ = tmplate_->xcell(0);
		expanded_.data_.substitute(*this);
		expanded_.Metrics(st);
		size_    = st;
		width_   = expanded_.width()   + 6;
		ascent_  = expanded_.ascent()  + 3;
		descent_ = expanded_.descent() + 3;
	}
}


void MathMacro::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);

	LColor::color col;

	if (mathcursor && mathcursor->isInside(this)) {

		int h = y - ascent() + 2 + expanded_.ascent();
		drawStr(pain, LM_TC_TEXTRM, size(), x + 3, h, name_);

		int const w = mathed_string_width(LM_TC_TEXTRM, size(), name_);
		expanded_.draw(pain, x + w + 12, h);
		h += expanded_.descent();

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(LM_TC_TEXTRM, size_, "#1: ", lasc, ldes, lwid);

		for (int i = 0; i < nargs(); ++i) {
			MathXArray & c = xcell(i);
			h += std::max(c.ascent(), lasc) + 5;
			c.draw(pain, x + lwid, h);
			char str[] = "#1:";
			str[1] += i;
			drawStr(pain, LM_TC_TEX, size(), x + 3, h, str);
			h += std::max(c.descent(), ldes) + 5;
		}
		col = LColor::red;
	} else {
		expanded_.draw(pain, x + 3, y);
		col = LColor::black;
	}

	pain.rectangle(x + 1, y - ascent() + 1, width() - 2, height() - 2, col);
}


void MathMacro::dump(std::ostream & os) const
{
	MathMacroTable::dump();
	os << "\n macro: '" << this << "'\n";
	os << " name: '" << name_ << "'\n";
	os << " template: '" << tmplate_ << "'\n";
	os << " template: '" << *tmplate_ << "'\n";
	os << endl;
}

void MathMacro::Write(std::ostream & os, bool fragile) const
{
	os << '\\' << name_;
	for (int i = 0; i < nargs(); ++i) {
		os << '{';
		cell(i).Write(os, fragile);
		os << '}';
	}
	if (nargs() == 0) 
		os << ' ';
}


void MathMacro::WriteNormal(std::ostream & os) const
{
	os << "[macro " << name_ << " ";
	for (int i = 0; i < nargs(); ++i) {
		cell(i).WriteNormal(os);
		os << ' ';
	}
	os << "] ";
}


bool MathMacro::idxUp(int & idx, int & pos) const
{
	return MathInset::idxLeft(idx, pos);
}


bool MathMacro::idxDown(int & idx, int & pos) const
{
	return MathInset::idxRight(idx, pos);
}


bool MathMacro::idxLeft(int &, int &) const
{
	return false;
}


bool MathMacro::idxRight(int &, int &) const
{
	return false;
}
