// -*- C++ -*-
/*
 *  File:        math_macro.C
 *  Purpose:     Implementation of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Mathed
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.2, Mathed & Lyx project.
 *
 *  This code is under the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macro.h"
#include "array.h"
#include "math_iter.h"
#include "math_inset.h"
#include "math_accentinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_rowst.h"
#include "support/lstrings.h"
#include "debug.h"
#include "mathed/support.h"
#include "math_macrotemplate.h"
#include "macro_support.h"
#include "Painter.h"

using namespace std;

ostream & operator<<(ostream & o, MathedTextCodes mtc)
{
	return o << int(mtc);
}


MathMacro::MathMacro(MathMacroTemplate const & t)
	: MathParInset(LM_ST_TEXT, t.GetName(), LM_OT_MACRO),
	  tmplate_(const_cast<MathMacroTemplate *>(&t)),
		args_(t.nargs()),
		idx_(t.nargs() ? 0 : -1)
{
	array = tmplate_->GetData();
	for (int i = 0; i < nargs(); ++i) 
		args_[i].reset(new MathParInset);
}


MathedInset * MathMacro::Clone()
{
	return new MathMacro(*this);
}


void MathMacro::expand()
{
	expanded_.reset(static_cast<MathParInset *>(tmplate_->Clone()));
	expanded_->substitute(this);
}



MathParInset const * MathMacro::arg(int i) const
{
	if (i < 0 || i >= nargs()) {
		lyxerr << "Illegal index " << i << " max: " << nargs() << endl;
		lyx::Assert(0);
		return 0;
	}
	return i >= 0 ? args_[i].get() : static_cast<MathParInset const *>(this);
}


MathParInset * MathMacro::arg(int i) 
{
	if (i < 0 || i >= nargs()) {
		lyxerr << "Illegal index " << i << " max: " << nargs() << endl;
		lyx::Assert(0);
		return 0;
	}
	return i >= 0 ? args_[i].get() : static_cast<MathParInset *>(this);
}


MathMacroTemplate * MathMacro::tmplate() const
{
	return const_cast<MathMacroTemplate *>(tmplate_);
}


extern bool is_mathcursor_inside(MathParInset *);

void MathMacro::Metrics()
{
	if (is_mathcursor_inside(this)) {
		tmplate_->Metrics();
		width   = tmplate_->Width()   + 4;
		ascent  = tmplate_->Ascent()  + 2;
		descent = tmplate_->Descent() + 2;

		width +=  mathed_string_width(LM_TC_TEXTRM, size(), GetName()) + 10;

		for (int i = 0; i < nargs(); ++i) {
			MathParInset * p = arg(i);
			p->Metrics();
			if (p->Width() + 30 > width) 
				width = p->Width() + 30;
			descent += p->Height() + 10;
		}
	} else {
		expand();
		expanded_->Metrics();
		width   = expanded_->Width()   + 6;
		ascent  = expanded_->Ascent()  + 3;
		descent = expanded_->Descent() + 3;
	}
}


void MathMacro::draw(Painter & pain, int x, int y)
{
	LColor::color col;

	if (is_mathcursor_inside(this)) {
		int h = y + Descent() - 2;
		for (int i = nargs() - 1; i >= 0; --i) {
			MathParInset * p = arg(i);
			h -= p->Descent() + 5;
			p->draw(pain, x + 30, h);
			char str[] = "#1:";
			str[1] += i;
			drawStr(pain, LM_TC_TEX, size(), x + 3, h, str);
			h -= p->Ascent() + 5;
		}

		h -= tmplate_->Descent();
		int w =  mathed_string_width(LM_TC_TEXTRM, size(), GetName());
		drawStr(pain, LM_TC_TEXTRM, size(), x + 3, h, GetName());
		tmplate_->draw(pain, x + w + 12, h);

		col = LColor::red;
	} else {
		expanded_->draw(pain, x + 3, y);
		col = LColor::black;
	}

	int w = Width();
	int a = Ascent();
	int h = Height();
	pain.rectangle(x + 1, y - a + 1, w - 2, h - 2, col);
}


bool MathMacro::setArgumentIdx(int i)
{
	if (i >= 0 && 0 < (nargs() - i)) {
		idx_ = i;
		return true;
	} else
		return false;
	idx_ = i;
	return true;
}


int MathMacro::getArgumentIdx() const 
{ 
	//lyxerr << "MathMacro::getArgumentIdx: res: " << idx_ << endl;
	return idx_;
}


int MathMacro::getMaxArgumentIdx() const 
{ 
	return nargs() - 1;
}



int MathMacro::nargs() const 
{ 
	return args_.size();
} 


MathedArray & MathMacro::GetData() 
{ 
	//lyxerr << "MathMacro::GetData: " << *this << endl;
	return idx_ >= 0 ? arg(idx_)->GetData() : MathParInset::GetData(); 
} 


MathedArray const & MathMacro::GetData() const
{ 
	//lyxerr << "MathMacro::GetData: " << *this << endl;
	return idx_ >= 0 ? arg(idx_)->GetData() : MathParInset::GetData(); 
} 


int MathMacro::GetColumns() const
{
	return idx_ >= 0 ? arg(idx_)->GetColumns() : MathParInset::GetColumns();
}


void MathMacro::GetXY(int & x, int & y) const
{
	if (idx_ >= 0)
		arg(idx_)->GetXY(x, y);
	else
		MathParInset::GetXY(x, y);
}


bool MathMacro::Permit(short f) const
{
	return idx_ >= 0 ? arg(idx_)->Permit(f) : MathParInset::Permit(f);
}


void MathMacro::SetFocus(int x, int y)
{
	idx_ = -1;
	for (int i = 0; i < nargs(); ++i) {
		if (arg(i)->Inside(x, y)) {
			idx_ = i;
			break;
		}
	}
}

void MathMacro::setData(MathedArray const & a, int i)
{
	arg(i)->setData(a);
}


void MathMacro::setData(MathedArray const & a)
{
	if (idx_ >= 0)
		arg(idx_)->setData(a);
	else
		array = a;
}


MathedTextCodes MathMacro::getTCode() const
{
	return nargs() ? LM_TC_ACTIVE_INSET : LM_TC_INSET;
	//return LM_TC_INSET;
}
	
void MathMacro::dump(ostream & os) const
{
	os << "\n macro: '" << this << "'\n";
	os << " name: '" << name << "'\n";
	os << " idx: '" << idx_ << "'\n";
	os << " data: '" << array << "'\n";
	os << " nargs: '" << nargs() << "'\n";
	for (int i = 0; i < nargs(); ++i)
		os << "  " << arg(i) << ": " << arg(i)->GetData() << endl;
	os << endl;
}

void MathMacro::Write(ostream & os, bool fragile)
{
	os << '\\' << name;
	for (int i = 0; i < nargs(); ++i) {
		os << '{';
		arg(i)->Write(os, fragile);
		os << '}';
	}
	if (nargs() == 0) 
		os << ' ';
}


void MathMacro::WriteNormal(ostream & os)
{
	os << "[macro " << name << " ";
	for (int i = 0; i < nargs(); ++i) {
		arg(i)->WriteNormal(os);
		os << ' ';
	}
	os << "] ";
}
