// -*- C++ -*-
/*
 *  File:        math_root.C
 *  Purpose:     Implementation of the root object 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: (c) 1999 Alejandro Aguilar Sierra
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

MathRootInset::MathRootInset(short st): MathSqrtInset(st)
{
    idx = 1;
    uroot = new MathParInset(LM_ST_TEXT); 
}


MathRootInset::~MathRootInset() 
{
    delete uroot;
}


MathedInset *MathRootInset::Clone()
{
   MathRootInset* p = new MathRootInset(GetStyle());
   MathedIter it(array), itr(uroot->GetData());
   p->SetData(it.Copy());
   p->setArgumentIdx(0);
   p->SetData(itr.Copy());

   return p;
}


void MathRootInset::SetData(LyxArrayBase *d)
{
   if (idx==1)
     MathParInset::SetData(d);
   else {
      uroot->SetData(d);
   }
}


bool MathRootInset::setArgumentIdx(int i)
{
   if (i==0 || i==1) {
       idx = i;
       return true;
   } else
      return false;
}


void MathRootInset::GetXY(int& x, int& y) const
{
   if (idx==1)
     MathParInset::GetXY(x, y);
   else
     uroot->GetXY(x, y);
}

LyxArrayBase *MathRootInset::GetData()
{
   if (idx==1)
     return array;
   else
     return uroot->GetData();
}


bool MathRootInset::Inside(int x, int y)
{
    return (uroot->Inside(x, y) || MathSqrtInset::Inside(x, y));
}


void MathRootInset::Metrics()
{
    int idxp = idx;

    idx = 1;
    MathSqrtInset::Metrics();
    uroot->Metrics();
    wroot = uroot->Width();
    dh = Height()/2;
    width += wroot;
    //    if (uroot->Ascent() > dh) 
    if (uroot->Height() > dh) 
      ascent += uroot->Height() - dh;
    dh -= descent - uroot->Descent();
    idx = idxp;
}


void MathRootInset::Draw(int x, int y)
{
    int idxp = idx;

    idx = 1;
    uroot->Draw(x, y - dh);
    MathSqrtInset::Draw(x+wroot, y);
    XFlush(fl_display);
    idx = idxp;
}


void MathRootInset::SetStyle(short st)
{
    MathSqrtInset::SetStyle(st);
    
    uroot->SetStyle((size<LM_ST_SCRIPTSCRIPT) ? size+1: size);
}


void MathRootInset::SetFocus(int x, int)
{  
    idx = (x > xo + wroot) ? 1: 0;
}


void MathRootInset::Write(FILE *outf)
{ 
   LString output;
   MathRootInset::Write(output);  
   fprintf(outf, "%s", output.c_str());
}


void MathRootInset::Write(LString &outf)
{ 
   outf += '\\';
   outf += name;
   outf += '[';
   uroot->Write(outf);  
   outf += "]{";
   MathParInset::Write(outf);
   outf += '}';
}
