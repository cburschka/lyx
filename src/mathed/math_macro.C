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
	MathParInset(LM_ST_TEXT, "", LM_OT_MACRO), tmplate(t)
{
	nargs = tmplate->getNoArgs();
	tcode = tmplate->getTCode();
	args_.resize(nargs);
	for (int i = 0; i < nargs; ++i) {
		args_[i].row = 0;
	}
	idx = 0;
	SetName(tmplate->GetName());
}


MathMacro::MathMacro(MathMacro * m): 
	MathParInset(LM_ST_TEXT, m->GetName(), LM_OT_MACRO)
{
	tmplate = m->tmplate;
	nargs = tmplate->getNoArgs();
	tcode = tmplate->getTCode();
	args_.resize(nargs);
	idx = 0;
	SetName(tmplate->GetName());
	for (int i = 0; i < tmplate->nargs; ++i) {
		m->setArgumentIdx(i);
		MathedIter it(m->GetData());
		args_[i].row   = m->args_[i].row;
		args_[i].array = it.Copy();
	}
}


MathMacro::~MathMacro()
{
	for (idx = 0; idx < nargs; ++idx) {
		MathedIter it(args_[idx].array);
		it.Clear();
		delete args_[idx].row;
	}
}


MathedInset * MathMacro::Clone()
{
	return new MathMacro(this);
}


void MathMacro::Metrics()
{
	if (nargs > 0)
		tmplate->update(this);
	tmplate->SetStyle(size);
	tmplate->Metrics();
	width = tmplate->Width();
	ascent = tmplate->Ascent();
	descent = tmplate->Descent();
}


void MathMacro::draw(Painter & pain, int x, int y)
{
	xo = x;
	yo = y;
	Metrics();
	tmplate->update(this);
	tmplate->SetStyle(size);
	tmplate->draw(pain, x, y);
	for (int i = 0; i < nargs; ++i) {
		tmplate->GetMacroXY(i, args_[i].x, args_[i].y);
	}
}


bool MathMacro::setArgumentIdx(int i)
{
	if (i >= 0 && i < nargs) {
		idx = i;
		return true;
	} else
		return false;
}


int MathMacro::getArgumentIdx() const 
{ 
	return idx; 
}


int MathMacro::getMaxArgumentIdx() const 
{ 
	return nargs - 1; 
} 


MathedArray * MathMacro::GetData() 
{ 
	return args_[idx].array; 
} 


int MathMacro::GetColumns() const
{
	return tmplate->getMacroPar(idx)->GetColumns();
}


void MathMacro::GetXY(int & x, int & y) const
{
	x = args_[idx].x;
	y = args_[idx].y;
}


bool MathMacro::Permit(short f) const
{
	return (nargs > 0) ?
		tmplate->getMacroPar(idx)->Permit(f) : MathParInset::Permit(f);
}


void MathMacro::SetFocus(int x, int y)
{
	tmplate->update(this);
	tmplate->SetMacroFocus(idx, x, y);
}


void MathMacro::SetData(MathedArray * a)
{
	args_[idx].array = a;
}




MathedRowSt * MathMacro::getRowSt() const
{
	return args_[idx].row;
}


MathedTextCodes MathMacro::getTCode() const
{
	return tcode;
}
	

void MathMacro::Write(ostream & os, bool fragile)
{
	if (tmplate->flags & MMF_Exp) {
		lyxerr[Debug::MATHED] << "Expand " << tmplate->flags
				      << ' ' << MMF_Exp << endl; 
		tmplate->update(this);
		tmplate->Write(os, fragile);
	} else {
		if (tmplate->flags & MMF_Env) {
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
	
		if (!(tmplate->flags & MMF_Env) && nargs > 0) 
			os << '{';
	
		for (int i = 0; i < nargs; ++i) {
			array = args_[i].array;
			MathParInset::Write(os, fragile);
			if (i < nargs - 1)  
				os << "}{";
		}   
		if (tmplate->flags & MMF_Env) {
			os << "\\end{"
			   << name
			   << '}';
		} else {
			if (nargs > 0) 
				os << '}';
			else
				os << ' ';
		}
	}
}
