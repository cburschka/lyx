/*
 *  File:        math_draw.C
 *  Purpose:     Interaction and drawing for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math drawing and interaction for a WYSIWYG math editor.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include FORMS_H_LOCATION
#include "math_cursor.h"
#include "math_parser.h"
#include "debug.h"
#include "lyxfont.h"
#include "Painter.h"

using std::endl;

extern LyXFont const mathed_get_font(short type, int size);
extern int mathed_char_width(short type, int style, byte c);
extern int mathed_string_width(short type, int style, string const &);
extern int mathed_string_height(short, int, string const &, int &, int &);
extern int mathed_char_height(short, int, byte, int &, int &);

void
MathSpaceInset::draw(Painter & pain, int x, int y)
{ 

// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};

// Sadly, HP-UX CC can't handle that kind of initialization.

   int xp[4];
   int yp[4];
   
   xp[0] = ++x;            yp[0] = y - 3;
   xp[1] = x;	           yp[1] = y;
   xp[2] = x + width - 2;  yp[2] = y;
   xp[3] = x + width - 2;  yp[3] = y - 3;

   pain.lines(xp, yp, 4, (space) ? LColor::latex : LColor::math);
}


void 
MathParInset::draw(Painter & pain, int x, int y)
{
   byte cx, cxp = 0;
   int xp = 0, ls;
   int asc = df_asc, des = 0;
   bool limits = false;
    
   xo = x;  yo = y; 
   if (!array || array->empty()) {
       if (array) {
	   MathedXIter data(this);
	   data.GetPos(x, y);
       }
       pain.rectangle(x, y - df_asc, df_width, df_asc, LColor::mathline);
      return;
   }  
   MathedXIter data(this);
   data.GoBegin();
   while (data.OK()) {
      data.GetPos(x, y);
      cx = data.GetChar();
      if (cx >= ' ') {
	 byte * s = data.GetString(ls);
	  drawStr(pain, data.FCode(), size, x, y, s, ls);
	  mathed_char_height(LM_TC_CONST, size, 'y', asc, des);
	  limits = false;
      } else {
	  if (cx == 0) break;
	 if (MathIsInset(cx)) {
	    int yy = y;
	    MathedInset * p = data.GetInset();
	    if (cx == LM_TC_UP) {
	       if (limits) {		  x -= (xp>p->Width()) ? p->Width()+(xp-p->Width())/2: xp;  
		  yy -= (asc + p->Descent()+4);
	       } else
		  yy -= (p->Descent()>asc) ? p->Descent()+4: asc;
	    } else
	    if (cx == LM_TC_DOWN) {
	       if (limits) {
		  x -= (xp>p->Width()) ? p->Width()+(xp-p->Width())/2: xp;
		  yy += des + p->Ascent() + 2;
	       } else
		 yy += des + p->Ascent()/2;
	    } else {
	       asc = p->Ascent();
	       des = p->Descent();
	    }
	    p->draw(pain, x, yy);
	    if (cx!= LM_TC_UP && cx!= LM_TC_DOWN) {
	       limits = p->GetLimits();
	       if (limits) xp = p->Width();
	    }
	    data.Next();
	 } else 
	   if (cx == LM_TC_TAB) {
	       if ((cxp == cx || cxp == LM_TC_CR || data.IsFirst())) { // && objtype == L
		       pain.rectangle(x, y - df_asc, df_width, df_asc,
				      LColor::mathline);
	       }
	      data.Next();
	      limits = false;
	   } else
	    if (cx == LM_TC_CR) {
		if (cxp == LM_TC_TAB || cxp == LM_TC_CR || data.IsFirst()) { //  && objtype == LM_OT_MATRIX) {
			pain.rectangle(x, y - df_asc, df_width, df_asc,
				       LColor::mathline);
		}
		data.Next();
		limits = false;
	    }
	 else {	 
		 lyxerr << "GMathed Error: Unrecognized code[" << cx
			<< "]" << endl;
	    break;
	 }
      }
      cxp = cx;
   }
   if (cxp == LM_TC_TAB || cxp == LM_TC_CR) { // && objtype == LM_OT_MATRIX) {
      data.GetPos(x, y);
      pain.rectangle(x, y - df_asc, df_width, df_asc, LColor::mathline);
   }
}


void 
MathParInset::Metrics()
{
    byte cx;
    byte cxp = 0;
    string s;
    int ls;
    int asc = df_asc;
    int des = 0;
    int tb = 0;
    int tab = 0;

    bool limits = false;
    
    ascent = df_asc;//mathed_char_height(LM_TC_VAR, size, 'I', asc, des); 
    width = df_width;
    descent = 0;
    if (!array) return;
    if (array->empty()) return;
    
    ascent = 0;
    MathedXIter data(this);
    data.GoBegin();
    while (data.OK()) {
	cx = data.GetChar();      
	if (cx >= ' ') {
	    s = reinterpret_cast<char *>(data.GetString(ls));
	    mathed_string_height(data.FCode(), size, s, asc, des);
	    if (asc > ascent) ascent = asc;
	    if (des > descent) descent = des;
	    limits = false;
	    mathed_char_height(LM_TC_CONST, size, 'y', asc, des);
	} else
	  if (MathIsInset(cx)) {
	      MathedInset * p = data.GetInset();
	      p->SetStyle(size);   
	      p->Metrics();
	      if (cx == LM_TC_UP) {
		  asc += (limits) ? p->Height() + 4: p->Ascent() + 
		    ((p->Descent()>asc) ? p->Descent() - asc + 4: 0);
	      } else
		if (cx == LM_TC_DOWN) {
		    des += ((limits) ? p->Height() + 4: p->Height() - p->Ascent() / 2);
		} else {
		    asc = p->Ascent();
		    des = p->Descent();
		}
	      if (asc > ascent) ascent = asc;
	      if (des > descent) descent = des;
	      if (cx!= LM_TC_UP && cx!= LM_TC_DOWN)
		limits = p->GetLimits();
	      data.Next();
	  } else 
	  if (cx == LM_TC_TAB) {
	      int x, y;
	      data.GetIncPos(x, y);
	      if (data.IsFirst() || cxp == LM_TC_TAB || cxp == LM_TC_CR) {
		  if (ascent<df_asc) ascent = df_asc;
		  tb = x;
	      }
	      data.setTab(x-tb, tab);
	      tb = x;
	      ++tab;
	      limits = false;                   
	      data.Next();
	  } else
	  if (cx == LM_TC_CR) {
	      if (tb > 0) {
		  int x, y;
		  data.GetIncPos(x, y);
		  if (data.IsFirst() || cxp == LM_TC_TAB || cxp == LM_TC_CR) {
		      if (ascent<df_asc) ascent = df_asc;
		      tb = x;
		  } 
		  data.setTab(x - tb, tab);
	      } else //if (GetColumns() == 1) 
		    {
		  int x, y;
		  data.GetIncPos(x, y);
		  data.setTab(x, tab);
		  if (ascent<df_asc) ascent = df_asc;
	      } 
	      tb = tab = 0;
	      data.subMetrics(ascent, descent);
	      ascent = df_asc;   
	      descent = 0;
	      data.Next();
	  } else {
		lyxerr << "Mathed Error: Unrecognized code[" << cx
		       << "]" << endl;
	    break;
	}       
	cxp = cx;
    }
    data.GetIncPos(width, ls);
    
    // No matter how simple is a matrix, it is NOT a subparagraph
    if (isMatrix()) {
	if (cxp == LM_TC_TAB) {
	    if (ascent<df_asc) ascent = df_asc;
	    data.setTab(0, tab);
	} else {
	  data.setTab(width - tb, tab);
	}
    }
    	  
    data.subMetrics(ascent, descent);
}


void
MathSqrtInset::draw(Painter & pain, int x, int y)
{ 
   MathParInset::draw(pain, x + hmax + 2, y); 
   int h = ascent;
   int d = descent;
   int h2 = Height() / 2;
   int w2 = (Height() > 4 * hmax) ? hmax : hmax / 2; 
   int xp[4], yp[4];
   xp[0] = x + hmax + wbody; yp[0] = y - h;
   xp[1] = x + hmax;         yp[1] = y - h;
   xp[2] = x + w2;           yp[2] = y + d;
   xp[3] = x;                yp[3] = y + d - h2;
   pain.lines(xp, yp, 4, LColor::mathline);
}


void
MathSqrtInset::Metrics()
{
   MathParInset::Metrics();
   ascent += 4;
   descent += 2;
   int a, b;
   hmax = mathed_char_height(LM_TC_VAR, size, 'I', a, b);
   if (hmax < 10) hmax = 10;
   wbody = width + 4;
   width += hmax + 4;
}


void
MathFracInset::draw(Painter & pain, int x, int y)
{ 
    short idxp = idx;
    short sizex = size;
    
    idx = 0;
    if (size == LM_ST_DISPLAY) ++size;
    MathParInset::draw(pain, x + (width - w0) / 2, y - des0);
    den->draw(pain, x + (width - w1) / 2, y + den->Ascent() + 2 - dh);
    size = sizex;
    if (objtype == LM_OT_FRAC)
	    pain.line(x + 2, y - dh, x + width - 4, y - dh, LColor::mathline);
    idx = idxp;
}


void
MathFracInset::Metrics()
{
    if (!dh) {
	int a, b;
	dh = mathed_char_height(LM_TC_CONST, size, 'I', a, b)/2;
    }
    short idxp = idx;
    short sizex = size; 
    idx = 0;
    if (size == LM_ST_DISPLAY) ++size; 
    MathParInset::Metrics();
    size = sizex;
    w0 = width;
    int as = Height() + 2 + dh;
    des0 = Descent() + 2 + dh;
    den->Metrics();  
    w1 = den->Width();   
    width = ((w0 > w1) ? w0: w1) + 12;
    ascent = as; 
    descent = den->Height()+ 2 - dh;
    idx = idxp;
}


void
MathBigopInset::draw(Painter & pain, int x, int y)
{
   string s;
   short t;
   
   if (sym < 256 || sym == LM_oint) {
      s += (sym == LM_oint) ? LM_int : sym;
      t = LM_TC_BSYM;
   } else {
      s = name;
      t = LM_TC_TEXTRM;
   }
   if (sym == LM_oint) {
	   pain.arc(x, y - 5 * width / 4, width, width, 0, 360*64,
		    LColor::mathline);
	   ++x;
   }
   pain.text(x, y, s, mathed_get_font(t, size));
}


void
MathBigopInset::Metrics()
{   
	char c;
	string s;
	short t;
	
	if (sym < 256 || sym == LM_oint) {
		c = (sym == LM_oint) ? LM_int: sym;
		s += c;
		t = LM_TC_BSYM;
	} else {
		s = name;
		t = LM_TC_TEXTRM;
	}
	mathed_string_height(t, size, s, ascent, descent);
	width = mathed_string_width(t, size, s);
	if (sym == LM_oint) width += 2;
}
