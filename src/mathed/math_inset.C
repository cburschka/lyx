/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "math_inset.h"
#endif

#include "math_iter.h"
#include "math_inset.h"
#include "symbol_def.h"


inline
char * strnew(char const * s)
{
   char * s1 = new char[strlen(s)+1];
   return strcpy(s1, s);
}


MathedInset::MathedInset(MathedInset * inset) 
{
   if (inset) {
      name = inset->GetName();
      objtype = inset->GetType();
      size = inset->GetStyle();
      width = inset->Width();
      ascent = inset->Ascent();
      descent = inset->Descent();
   } else {
      objtype = LM_OT_UNDEF;
      size = LM_ST_TEXT;
      width = ascent = descent = 0;
      name = 0;
   }
}


MathFuncInset::MathFuncInset(char const * nm, short ot, short st)
	: MathedInset("", ot, st)
{
   ln = 0;
   lims = (GetType() == LM_OT_FUNCLIM);
    if (GetType() == LM_OT_UNDEF) {
	fname = strnew(nm);
	SetName(fname);
    } else {
	fname = 0;
	SetName(nm);
    }
}


MathedInset * MathFuncInset::Clone()
{
   return new MathFuncInset(name, GetType(), GetStyle());
}


MathSpaceInset::MathSpaceInset(int sp, short ot, short st)
	: MathedInset("", ot, st), space(sp)
{}


MathedInset * MathSpaceInset::Clone()
{
   return new MathSpaceInset(space, GetType(), GetStyle());
}


MathParInset::MathParInset(short st, char const * nm, short ot)
	: MathedInset(nm, ot, st)
{
    array = 0;
    ascent = 8;
    width = 4;
    descent = 0;
    flag = 1;
    if (objtype == LM_OT_SCRIPT)
      flag |= LMPF_SCRIPT;
}


MathParInset::MathParInset(MathParInset * p)
	: MathedInset(p)
{
    flag = p->flag;
    p->setArgumentIdx(0);
    MathedIter it(p->GetData());
    SetData(it.Copy());
}


MathParInset::~MathParInset()
{
   if (array) {
      MathedIter it(array);
      it.Clear();
      delete array;
   }
}


MathedInset * MathParInset::Clone()
{
   return new MathParInset(this);
}


void MathParInset::SetData(LyxArrayBase * a)
{
    array = a;
   
    // A standard paragraph shouldn't have any tabs nor CRs.
    if (array) {
	MathedIter it(array);
	while (it.OK()) {
	    char c = it.GetChar();
	    if (c == LM_TC_TAB || c == LM_TC_CR) 
	      it.Delete();
	    else
	      it.Next();
	}
   }
}


MathSqrtInset::MathSqrtInset(short st)
	: MathParInset(st, "sqrt", LM_OT_SQRT) {}


MathedInset * MathSqrtInset::Clone()
{   
   MathSqrtInset * p = new MathSqrtInset(GetStyle());
   MathedIter it(array);
   p->SetData(it.Copy());
   return p;
}


bool MathSqrtInset::Inside(int x, int y) 
{
	return x >= xo - hmax
		&& x <= xo + width - hmax
		&& y <= yo + descent
		&& y >= yo - ascent;
}


MathDelimInset::MathDelimInset(int l, int r, short st)
	: MathParInset(st, "", LM_OT_DELIM), left(l), right(r) {}


MathedInset * MathDelimInset::Clone()
{   
   MathDelimInset * p = new MathDelimInset(left, right, GetStyle());
   MathedIter it(array);
   p->SetData(it.Copy());
   return p;
}


MathDecorationInset::MathDecorationInset(int d, short st)
	: MathParInset(st, "", LM_OT_DECO), deco(d)
{
   upper = (deco!= LM_underline && deco!= LM_underbrace);
}


MathedInset * MathDecorationInset::Clone()
{   
   MathDecorationInset * p = new MathDecorationInset(deco, GetStyle());
   MathedIter it(array);
   p->SetData(it.Copy());
   return p;
}


MathFracInset::MathFracInset(short ot)
	: MathParInset(LM_ST_TEXT, "frac", ot)
{
	
    den = new MathParInset(LM_ST_TEXT); // this leaks
    dh = 0;
    idx = 0;
    if (objtype == LM_OT_STACKREL) {
	flag |= LMPF_SCRIPT;
	SetName("stackrel");
    }
}


MathFracInset::~MathFracInset()
{
    delete den;
}


MathedInset * MathFracInset::Clone()
{   
    MathFracInset * p = new MathFracInset(GetType());
    MathedIter itn(array);
    MathedIter itd(den->GetData());
    p->SetData(itn.Copy(), itd.Copy());
    p->idx = idx;
    p->dh = dh;
   return p;
}


bool MathFracInset::setArgumentIdx(int i)
{
   if (i == 0 || i == 1) {
       idx = i;
       return true;
   } else 
      return false;
}


void MathFracInset::SetStyle(short st)
{
    MathParInset::SetStyle(st);
    dh = 0;
    den->SetStyle((size == LM_ST_DISPLAY) ?
		  static_cast<short>(LM_ST_TEXT)
		  : size);
}


void MathFracInset::SetData(LyxArrayBase * n, LyxArrayBase * d)
{
   den->SetData(d);
   MathParInset::SetData(n);
}


void MathFracInset::SetData(LyxArrayBase * d)
{
   if (idx == 0)
     MathParInset::SetData(d);
   else {
      den->SetData(d);
   }
}


void MathFracInset::GetXY(int & x, int & y) const
{  
   if (idx == 0)
     MathParInset::GetXY(x, y);
   else
     den->GetXY(x, y);
}


LyxArrayBase * MathFracInset::GetData()
{
   if (idx == 0)
     return array;
   else
     return den->GetData();
}


bool MathFracInset::Inside(int x, int y) 
{
    int xx = xo - (width - w0) / 2;
    
    return x >= xx && x <= xx + width && y <= yo + descent && y >= yo - ascent;
}


void MathFracInset::SetFocus(int /*x*/, int y)
{  
//    lyxerr << "y " << y << " " << yo << " " << den->yo << " ";
    idx = (y > yo) ? 1: 0;
}


MathMatrixInset::MathMatrixInset(int m, int n, short st)
	: MathParInset(st, "array", LM_OT_MATRIX), nc(m)
{
    ws = new int[nc]; 
    v_align = 0;
    h_align = new char[nc + 1];
    for (int i = 0; i < nc; ++i) h_align[i] = 'c'; 
    h_align[nc] = '\0';
    nr = 0;
    row = 0;
    flag = 15;
    if (n > 0) {
	    row = new MathedRowSt(nc+1);
	MathedXIter it(this);
	for (int j = 1; j < n; ++j) it.addRow();
	nr = n;
	if (nr == 1 && nc > 1) {
	    for (int j = 0; j < nc - 1; ++j) 
	      it.Insert('T', LM_TC_TAB);
	}
    } else if (n < 0) {
	    row = new MathedRowSt(nc + 1);
	nr = 1;
    }
}


MathMatrixInset::MathMatrixInset(MathMatrixInset * mt)
	: MathParInset(mt->GetStyle(), mt->GetName(), mt->GetType())
{
	nr = 0;
    nc = mt->nc;
    ws = new int[nc];
    h_align = new char[nc + 1];
    strcpy(h_align, mt->GetAlign(&v_align));
    MathedIter it;
    it.SetData(mt->GetData());
    array = it.Copy();
    if (mt->row != 0) {
	MathedRowSt * r, * ro= 0, * mrow = mt->row;
	//mrow = mt->row; // This must be redundant...
	while (mrow) {
	    r = new MathedRowSt(nc + 1);
	    r->numbered = mrow->numbered;
	    if (mrow->label) 
	      r->label = strnew(mrow->label);
	    if (!ro) 
	      row = r;
	    else
	      ro->next = r;
	    mrow = mrow->next;
	    ro = r;
	    ++nr;
	} 
    } else         
      row = 0;
    flag = mt->flag;
}


MathMatrixInset::~MathMatrixInset()
{
    delete[] ws;
    
    MathedRowSt * r = row;
    while (r) {
	MathedRowSt * q = r->next;
	delete r;
	r = q;
    }
}


MathedInset * MathMatrixInset::Clone()
{
    return new MathMatrixInset(this);
}


void MathMatrixInset::SetAlign(char vv, char const * hh)
{
   v_align = vv;
   strncpy(h_align, hh, nc);
}


// Check the number of tabs and crs
void MathMatrixInset::SetData(LyxArrayBase * a)
{
    if (!a) return;
    MathedIter it(a);
    int nn = nc - 1;
    nr = 1;
    // count tabs per row
    while (it.OK()) {
	if (it.IsTab()) {
	    if (nn < 0) { 
		it.Delete();
		continue;
	    } else {
//	      it.Next();
		--nn;
	    }
	}
	if (it.IsCR()) {
	    while (nn > 0) {
		it.Insert(' ', LM_TC_TAB);
		--nn;
	    }
	    nn = nc - 1;
	    ++nr;
	}
	it.Next();
    }
    it.Reset();

    // Automatically inserts tabs around bops
    // DISABLED because it's very easy to insert tabs 
    array = a;
}


void MathMatrixInset::Draw(int x, int baseline)
{
    MathParInset::Draw(x, baseline);
}                


void MathMatrixInset::Metrics()
{
    int i, hl, h= 0;
    MathedRowSt * cprow= 0, * cxrow;

    if (!row) {
//	lyxerr << " MIDA ";
	MathedXIter it(this);
	row = it.adjustVerticalSt();
    } 
    
    // Clean the arrays      
    cxrow = row;
    while (cxrow) {   
	for (i = 0; i <= nc; ++i) cxrow->w[i] = 0;
	cxrow = cxrow->next;
    }
    
    // Basic metrics
    MathParInset::Metrics();
	    
    if (nc <= 1 && !row->next) {
	row->asc = ascent;
	row->desc = descent;
    }
    
    // Vertical positions of each row
    cxrow = row;     
    while (cxrow) {
	for (i = 0; i < nc; ++i) {
	    if (cxrow == row || ws[i]<cxrow->w[i]) ws[i]= cxrow->w[i];
	    if (cxrow->next == 0 && ws[i] == 0) ws[i] = df_width;
	}
	
	cxrow->y = (cxrow == row) ? cxrow->asc:
	           cxrow->asc + cprow->desc + MATH_ROWSEP + cprow->y;
	h += cxrow->asc + cxrow->desc + MATH_ROWSEP; 	
	cprow = cxrow;
	cxrow = cxrow->next;
    }
    
    hl = Descent();
    h -= MATH_ROWSEP;

    //  Compute vertical align
    switch (v_align) {
     case 't': ascent = row->y; break;
     case 'b': ascent = h - hl; break;
     default:  ascent = (row->next) ? h / 2: h - hl; break;
    }
    descent = h - ascent + 2;
    
   
   // Adjust local tabs
    cxrow = row;
    width = MATH_COLSEP;
    while (cxrow) {   
	int rg = MATH_COLSEP, ww, lf = 0, * w = cxrow->w;
	for (i = 0; i < nc; ++i) {
	    bool isvoid = false;
	    if (w[i] <= 0) {
		w[i] = df_width;
		isvoid = true;
	    }
	    switch (h_align[i]) {
	     case 'l': lf = 0; break;
	     case 'c': lf = (ws[i] - w[i])/2; 
		       break;
	     case 'r': lf = ws[i] - w[i]; break;
	    }
	    ww = (isvoid) ? lf: lf + w[i];
	    w[i] = lf + rg;
	    rg = ws[i] - ww + MATH_COLSEP;
	    if (cxrow == row) width += ws[i] + MATH_COLSEP;
	}
	cxrow->y -= ascent;
	cxrow = cxrow->next;
    }
}


MathAccentInset::MathAccentInset(byte cx, MathedTextCodes f, int cd, short st)
	: MathedInset("", LM_OT_ACCENT, st), c(cx), fn(f), code(cd)
{
    inset = 0;
}


MathAccentInset::MathAccentInset(MathedInset *ins, int cd, short st)
	: MathedInset("", LM_OT_ACCENT, st),
	  c(0), fn(LM_TC_MIN), code(cd), inset(ins) {}


MathAccentInset::~MathAccentInset()
{
    if (inset) // DEL LINE
      delete inset;
}


MathedInset * MathAccentInset::Clone()
{   
    MathAccentInset * p;
    
    if (inset) 
      p = new MathAccentInset(inset->Clone(), code, GetStyle());
    else
      p = new MathAccentInset(c, fn, code, GetStyle());
    
    return p;
}


MathBigopInset::MathBigopInset(char const* nam, int id, short st)
	: MathedInset(nam, LM_OT_BIGOP, st), sym(id)
{
   lims = -1;
}


MathedInset * MathBigopInset::Clone()
{
   return new MathBigopInset(name, sym, GetStyle());
}


MathDotsInset::MathDotsInset(char const * nam, int id, short st)
	: MathedInset(nam, LM_OT_DOTS, st), code(id) {}


MathedInset * MathDotsInset::Clone()
{
   return new MathDotsInset(name, code, GetStyle());
}     
