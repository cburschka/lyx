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


MathMacro::MathMacro(boost::shared_ptr<MathMacroTemplate> const & t)
	: MathParInset(LM_ST_TEXT, "", LM_OT_MACRO),
	  tmplate_(t)
{
	//nargs_ = tmplate_->getNoArgs();
	int const n = tmplate_->getNoArgs();
	
	tcode_ = tmplate_->getTCode();

	for (int i = 0; i < n; ++i) {
		args_.push_back(MathMacroArgument(t->args_[i]));
	}
	//for (int i = 0; i < nargs_; ++i) {
	//	MathMacroArgument * ma = new MathMacroArgument(*t->args_[i]);
	//	args_.push_back(boost::shared_ptr<MathMacroArgument>(ma));
	//}
	
	idx_ = 0;
	SetName(tmplate_->GetName());
}


MathedInset * MathMacro::Clone()
{
	return new MathMacro(*this);
}


void MathMacro::Metrics()
{
	if (args_.size() > 0)
		tmplate_->update(*this);
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
	tmplate_->SetStyle(size());
	tmplate_->draw(pain, x, y);
}


bool MathMacro::setArgumentIdx(int i)
{
	if (i >= 0 && i < args_.size()) {
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
	return args_.size() - 1;
} 


MathedArray & MathMacro::GetData() 
{ 
	return args_[idx_].GetData(); 
} 


MathedArray const & MathMacro::GetData() const
{ 
	return args_[idx_].GetData(); 
} 


int MathMacro::GetColumns() const
{
	return tmplate_->getMacroPar(idx_)->GetColumns();
}


void MathMacro::GetXY(int & x, int & y) const
{
	const_cast<MathMacro*>(this)->Metrics();
	tmplate_->GetMacroXY(idx_, x, y);
}


bool MathMacro::Permit(short f) const
{
	return (args_.size() > 0) ?
		tmplate_->getMacroPar(idx_)->Permit(f) :
		MathParInset::Permit(f);
}


void MathMacro::SetFocus(int x, int y)
{
	Metrics();
	tmplate_->SetMacroFocus(idx_, x, y);
}


void MathMacro::setData(MathedArray const & a)
{
	args_[idx_].setData(a);
}


MathedTextCodes MathMacro::getTCode() const
{
	return tcode_;
}
	

void MathMacro::Write(ostream & os, bool fragile)
{
	os << '\\' << name;

	int const n = args_.size();
	
	if (n > 0) {
		os << '{';
		
		for (int i = 0; i < n; ++i) {
			array = args_[i].GetData();
			MathParInset::Write(os, fragile);
			if (i < n - 1)  
				os << "}{";
		}
		os << '}';
	} else
		os << ' ';
}


MathMacroArgument const & MathMacro::getArg(int i) const
{
	return args_[i];
}
//boost::shared_ptr<MathMacroArgument> MathMacro::getArg(int i)
//{
//	return args_[i];
//}

