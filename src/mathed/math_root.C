// -*- C++ -*-
/*
 *  File:        math_root.C
 *  Purpose:     Implementation of the root object 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: 1999 Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_iter.h"
#include "math_root.h"
#include "support/LOstream.h"

using std::ostream;

MathRootInset::MathRootInset(short st)
	: MathSqrtInset(st)
{
	idx_ = 1;
	uroot_ = new MathParInset(LM_ST_TEXT); 
}


MathRootInset::~MathRootInset() 
{
	delete uroot_;
}


MathedInset * MathRootInset::Clone()
{
	MathRootInset * p = new MathRootInset(*this);
	p->uroot_ = (MathParInset *) p->uroot_->Clone();
	p->setArgumentIdx(0);
	return p;
}


void MathRootInset::setData(MathedArray * d)
{
	if (idx_ == 1)
		MathParInset::setData(d);
	else {
		uroot_->setData(d);
	}
}


bool MathRootInset::setArgumentIdx(int i)
{
	if (i == 0 || i == 1) {
		idx_ = i;
		return true;
	} else
		return false;
}


void MathRootInset::GetXY(int & x, int & y) const
{
	if (idx_ == 1)
		MathParInset::GetXY(x, y);
	else
		uroot_->GetXY(x, y);
}


MathedArray * MathRootInset::GetData()
{
	if (idx_ == 1)
		return &array;
	else
		return uroot_->GetData();
}


bool MathRootInset::Inside(int x, int y)
{
	return (uroot_->Inside(x, y) || MathSqrtInset::Inside(x, y));
}


void MathRootInset::Metrics()
{
	int idxp = idx_;
	
	idx_ = 1;
	MathSqrtInset::Metrics();
	uroot_->Metrics();
	wroot_ = uroot_->Width();
	dh_ = Height()/2;
	width += wroot_;
	//    if (uroot->Ascent() > dh) 
	if (uroot_->Height() > dh_) 
		ascent += uroot_->Height() - dh_;
	dh_ -= descent - uroot_->Descent();
	idx_ = idxp;
}


void MathRootInset::draw(Painter & pain, int x, int y)
{
	int idxp = idx_;
	
	idx_ = 1;
	uroot_->draw(pain, x, y - dh_);
	MathSqrtInset::draw(pain, x + wroot_, y);
	idx_ = idxp;
}


void MathRootInset::SetStyle(short st)
{
	MathSqrtInset::SetStyle(st);
	
	uroot_->SetStyle((size() < LM_ST_SCRIPTSCRIPT) ? size() + 1 : size());
}


void MathRootInset::SetFocus(int x, int)
{  
	idx_ = (x > xo() + wroot_) ? 1: 0;
}


void MathRootInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '[';
	uroot_->Write(os, fragile);  
	os << "]{";
	MathParInset::Write(os, fragile);
	os << '}';
}
