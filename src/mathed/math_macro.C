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

using std::ostream;
using std::endl;

ostream & operator<<(ostream & o, MathedTextCodes mtc)
{
	return o << int(mtc);
}


MathMacro::MathMacro(MathMacroTemplate * t): 
	MathParInset(LM_ST_TEXT, "", LM_OT_MACRO), tmplate_(t)
{
	nargs_ = tmplate_->getNoArgs();
	tcode_ = tmplate_->getTCode();
	args_.resize(nargs_);
	for (int i = 0; i < nargs_; ++i) {
		args_[i].row = 0;
	}
	idx_ = 0;
	SetName(tmplate_->GetName());
}


MathMacro::MathMacro(MathMacro * m): 
	MathParInset(LM_ST_TEXT, m->GetName(), LM_OT_MACRO)
{
	tmplate_ = m->tmplate_;
	nargs_ = tmplate_->getNoArgs();
	tcode_ = tmplate_->getTCode();
	args_.resize(nargs_);
	idx_ = 0;
	SetName(tmplate_->GetName());
	for (int i = 0; i < tmplate_->getNoArgs(); ++i) {
		m->setArgumentIdx(i);
		args_[i].row   = m->args_[i].row;
		args_[i].array = m->GetData();
	}
}


MathMacro::~MathMacro()
{
}


MathedInset * MathMacro::Clone()
{
	return new MathMacro(this);
}


void MathMacro::Metrics()
{
	if (nargs_ > 0)
		tmplate_->update(this);
	tmplate_->SetStyle(size());
	tmplate_->Metrics();
	width = tmplate_->Width();
	ascent = tmplate_->Ascent();
	descent = tmplate_->Descent();
}


void MathMacro::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);
	Metrics();
	tmplate_->update(this);
	tmplate_->SetStyle(size());
	tmplate_->draw(pain, x, y);
	for (int i = 0; i < nargs_; ++i) {
		tmplate_->GetMacroXY(i, args_[i].x, args_[i].y);
	}
}


bool MathMacro::setArgumentIdx(int i)
{
	if (i >= 0 && i < nargs_) {
		idx_ = i;
		return true;
	} else
		return false;
}


int MathMacro::getArgumentIdx() const 
{ 
	return idx_;
}


int MathMacro::getMaxArgumentIdx() const 
{ 
	return nargs_ - 1; 
} 


MathedArray & MathMacro::GetData() 
{ 
	return args_[idx_].array; 
} 


int MathMacro::GetColumns() const
{
	return tmplate_->getMacroPar(idx_)->GetColumns();
}


void MathMacro::GetXY(int & x, int & y) const
{
	x = args_[idx_].x;
	y = args_[idx_].y;
}


bool MathMacro::Permit(short f) const
{
	return (nargs_ > 0) ?
		tmplate_->getMacroPar(idx_)->Permit(f) : MathParInset::Permit(f);
}


void MathMacro::SetFocus(int x, int y)
{
	tmplate_->update(this);
	tmplate_->SetMacroFocus(idx_, x, y);
}


void MathMacro::setData(MathedArray const & a)
{
	args_[idx_].array = a;
}




MathedRowSt * MathMacro::getRowSt() const
{
	return args_[idx_].row;
}


MathedTextCodes MathMacro::getTCode() const
{
	return tcode_;
}
	

void MathMacro::Write(ostream & os, bool fragile)
{
	if (tmplate_->flags() & MMF_Exp) {
		lyxerr[Debug::MATHED] << "Expand " << tmplate_->flags()
				      << ' ' << MMF_Exp << endl; 
		tmplate_->update(this);
		tmplate_->Write(os, fragile);
	} else {
		if (tmplate_->flags() & MMF_Env) {
			os << "\\begin{"
			   << name
			   << "} ";
		} else {
			os << '\\' << name;
		}
//	if (options) { 
//	  file += '[';
//	  file += options;
//	  file += ']';
//      }
	
		if (!(tmplate_->flags() & MMF_Env) && nargs_ > 0) 
			os << '{';
	
		for (int i = 0; i < nargs_; ++i) {
			array = args_[i].array;
			MathParInset::Write(os, fragile);
			if (i < nargs_ - 1)  
				os << "}{";
		}   
		if (tmplate_->flags() & MMF_Env) {
			os << "\\end{"
			   << name
			   << '}';
		} else {
			if (nargs_ > 0) 
				os << '}';
			else
				os << ' ';
		}
	}
}
