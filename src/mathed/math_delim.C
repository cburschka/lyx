 /* 
 *  File:        math_delim.C
 *  Purpose:     Draw delimiters and decorations
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Vectorial fonts for simple and resizable objets.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include FORMS_H_LOCATION
#include <cstdlib>
#include "symbol_def.h"
#include "math_inset.h"
#include "LColor.h"
#include "Painter.h"

/* 
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4= square polyline
 */

static float parenth[] = {
  2.0, 13.0,
  0.9930, 0.0071,  0.7324, 0.0578,  0.5141, 0.1126,  0.3380, 0.1714,
  0.2183, 0.2333,  0.0634, 0.3621,  0.0141, 0.5000,  0.0563, 0.6369,
  0.2113, 0.7647,  0.3310, 0.8276,  0.5070, 0.8864,  0.7254, 0.9412,
  0.9930, 0.9919,
  0.0   
};

static float parenthHigh[] = {
    2.0, 13.0, 0.9840, 0.0014, 0.7143, 0.0323, 0.4603, 0.0772, 0.2540, 
    0.1278, 0.1746, 0.1966, 0.0952, 0.3300, 0.0950, 0.5000, 0.0952, 0.6700, 
    0.1746, 0.8034, 0.2540, 0.8722, 0.4603, 0.9228, 0.7143, 0.9677, 0.9840, 
    0.9986, 0.0 
};

static float brace[] = {
  2.0, 21.0,
  0.9492, 0.0020, 0.9379, 0.0020, 0.7458, 0.0243, 0.5819, 0.0527,
  0.4859, 0.0892, 0.4463, 0.1278, 0.4463, 0.3732, 0.4011, 0.4199,
  0.2712, 0.4615, 0.0734, 0.4919, 0.0113, 0.5000, 0.0734, 0.5081,
  0.2712, 0.5385, 0.4011, 0.5801, 0.4463, 0.6268, 0.4463, 0.8722,
  0.4859, 0.9108, 0.5819, 0.9473, 0.7458, 0.9757, 0.9379, 0.9980,
  0.9492, 0.9980,
  0.0
};

static float arrow[] = {
   4, 7,
   0.015, 0.7500,  0.2, 0.6,  0.35, 0.35,  0.5, 0.05,
   0.65, 0.35,  0.8, 0.6,  0.95, 0.7500,
   3, 0.5, 0.15,  0.5, 0.95,
   0.0 
};

static float Arrow[] = {
   4, 7,
   0.015, 0.7500,  0.2, 0.6,  0.35, 0.35,  0.5, 0.05,
   0.65, 0.35,  0.8, 0.6,  0.95, 0.7500,
   3, 0.35, 0.5, 0.35, 0.95,
   3, 0.65, 0.5, 0.65, 0.95,
   0.0
};

static float udarrow[] = {
   2, 3,
   0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
   2, 3,
   0.015, 0.75,  0.5, 0.95, 0.95, 0.75,  
   1, 0.5, 0.2,  0.5, 0.8,
   0.0 
};

static float Udarrow[] = {
   2, 3,
   0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
   2, 3,
   0.015, 0.75,  0.5, 0.95, 0.95, 0.75,  
   1, 0.35, 0.2, 0.35, 0.8,
   1, 0.65, 0.2, 0.65, 0.8,
   0.0 
};

static float brack[] = {
   2.0, 4,
   0.95, 0.05,  0.05, 0.05,  0.05, 0.95,  0.95, 0.95,
   0.0
};

static float corner[] = {
   2.0, 3,
   0.95, 0.05,  0.05, 0.05,  0.05, 0.95,
   0.0
};

static float angle[] = {
   2.0, 3,
   1, 0,  0.05, 0.5,  1, 1,
   0.0
};

static float slash[] = {
   1, 0.95, 0.05,  0.05, 0.95, 
   0.0
};

static float hline[] = {
   1, 0.05, 0.5,  0.95, 0.5, 
   0.0
};


static float hline2[] = {
   1, 0.1, 0.5,  0.3, 0.5,
   1, 0.7, 0.5,  0.9, 0.5,
   0.0
}; 

static float hline3[] = {
   1, 0.1, 0,  0.15, 0,
   1, 0.475, 0,  0.525, 0,
   1, 0.85, 0,  0.9, 0,  
   0.0
};


static float dline3[] = {
   1, 0.1, 0.1,  0.15, 0.15,
   1, 0.475, 0.475,  0.525, 0.525,
   1, 0.85, 0.85,  0.9, 0.9,
   0.0
};     

static float hlinesmall[] = {
   1, 0.4, 0.5,  0.6, 0.5, 
   0.0
};

static float vert[] = {
   1, 0.5, 0.05,  0.5, 0.95, 
   0.0
};

static float Vert[] = {
   1, 0.3, 0.05,  0.3, 0.95, 
   1, 0.7, 0.05,  0.7, 0.95,
   0.0
};

static float tilde[] = {
   2.0, 4,
   0.05, 0.8,  0.25, 0.2,  0.75, 0.8,  0.95, 0.2,
   0.0
};

static struct math_deco_struct {
    int code; float *data; int angle;
} math_deco_table[] = {   

   // Decorations
  { LM_widehat, &angle[0], 3 },
  { LM_widetilde, &tilde[0], 0 },
  { LM_underline, &hline[0], 0 },
  { LM_overline, &hline[0], 0 },
  { LM_underbrace, &brace[0], 1 },
  { LM_overbrace,  &brace[0], 3 },
  { LM_overleftarrow, &arrow[0], 1 },
  { LM_overightarrow, &arrow[0], 3 },
     
  // Delimiters
  { '(', &parenth[0], 0 },
  { ')', &parenth[0], 2 },
  { '{', &brace[0], 0 },
  { '}', &brace[0], 2 },
  { '[', &brack[0], 0 },
  { ']', &brack[0], 2 },
  { '|', &vert[0], 0 },
  { '/', &slash[0], 0 },
  { LM_Vert, &Vert[0], 0 },
  { LM_backslash, &slash[0], 1 },
  { LM_langle, &angle[0], 0 },
  { LM_lceil, &corner[0], 0 }, 
  { LM_lfloor, &corner[0], 1 },  
  { LM_rangle, &angle[0], 2 }, 
  { LM_rceil, &corner[0], 3 }, 
  { LM_rfloor, &corner[0], 2 },
  { LM_downarrow, &arrow[0], 2 },
  { LM_Downarrow, &Arrow[0], 2 }, 
  { LM_uparrow, &arrow[0], 0 },
  { LM_Uparrow, &Arrow[0], 0 },
  { LM_updownarrow, &udarrow[0], 0 },
  { LM_Updownarrow, &Udarrow[0], 0 },	 

  // Accents   
  { LM_ddot, &hline2[0], 0 },
  { LM_hat, &angle[0], 3 },
  { LM_grave, &slash[0], 1 },
  { LM_acute, &slash[0], 0 },
  { LM_tilde, &tilde[0], 0 },
  { LM_bar, &hline[0], 0 },
  { LM_dot, &hlinesmall[0], 0 },
  { LM_check, &angle[0], 1 },
  { LM_breve, &parenth[0], 1 },
  { LM_vec, &arrow[0], 3 },
  { LM_not, &slash[0], 0 },  

  // Dots
  { LM_ldots, &hline3[0], 0 }, 
  { LM_cdots, &hline3[0], 0 },
  { LM_vdots, &hline3[0], 1 },
  { LM_ddots, &dline3[0], 0 },
     
  { 0, 0, 0 }
};


inline int odd(int x) { return ((x) & 1); }

typedef float matriz_data[2][2];

const matriz_data MATIDEN= { {1, 0}, {0, 1}};

#ifndef USE_PAINTER
extern void mathed_set_font(short type, int style);
#endif
extern int mathed_char_width(short type, int style, byte c);
extern int mathed_char_height(short, int, byte, int&, int&);

#define mateq(m1, m2)  memcpy(m1, m2, sizeof(matriz_data))

class Matriz {
 public: 
   Matriz() { mateq(m, MATIDEN); }
   void rota(int);
   void escala(float, float);
   void transf(float, float, float&, float&);
   
 protected:
   matriz_data m;
   void matmat(matriz_data& a);
};


void Matriz::rota(int code)
{
   float cs, sn;
   
   matriz_data r;
   mateq(r, MATIDEN);
   cs = (odd(code)) ? 0: 1 - code;
   sn = (odd(code)) ? 2 - code: 0;
   r[0][0] = cs;         r[0][1] = sn;
   r[1][0] = -r[0][1];   r[1][1] = r[0][0];
   matmat(r);
}

void Matriz::escala(float x, float y)
{
   matriz_data s;
   mateq(s, MATIDEN);
   s[0][0] = x;  s[1][1] = y;
   matmat(s);
}


void Matriz::matmat(matriz_data& a)
{
   matriz_data c;   
   for (int i = 0;i < 2; ++i) {
      c[0][i] = a[0][0]*m[0][i] + a[0][1]*m[1][i];
      c[1][i] = a[1][0]*m[0][i] + a[1][1]*m[1][i];
   }
   mateq(m, c);
}

void Matriz::transf(float xp, float yp, float &x, float &y)
{
   x = m[0][0]*xp + m[0][1]*yp;
   y = m[1][0]*xp + m[1][1]*yp;
}

#ifndef USE_PAINTER
extern GC latexGC, mathGC, mathLineGC, cursorGC;
#endif

static int search_deco(int code)
{
   int i= 0;
   
   while (math_deco_table[i].code &&  math_deco_table[i].code!= code) ++i;
   if (!math_deco_table[i].code) i = -1;
   return i;
}


#ifdef USE_PAINTER
void mathed_draw_deco(Painter & pain, int x, int y, int w, int h, int code)
{
	Matriz mt, sqmt;
	float xx, yy, x2, y2;
	int i = 0;
   
	int j = search_deco(code);   
	if (j < 0) return;
   
	int r = math_deco_table[j].angle;
	float * d = math_deco_table[j].data;
	
	if (h > 70 && (math_deco_table[j].code == int('(') || math_deco_table[j].code == int(')')))
		d = parenthHigh;
	
	mt.rota(r);
	mt.escala(w, h);
   
	int n = (w < h) ? w: h;
	sqmt.rota(r);
	sqmt.escala(n, n);
	if (r > 0 && r < 3) y += h;   
	if (r >= 2) x += w;   
	do {
		code = int(d[i++]);
		switch (code) {
		case 0: break;
		case 1: 
		case 3:
		{
			xx = d[i++]; yy = d[i++];
			x2 = d[i++]; y2 = d[i++];
			if (code == 3) 
				sqmt.transf(xx, yy, xx, yy);
			else
				mt.transf(xx, yy, xx, yy);
			mt.transf(x2, y2, x2, y2);
			pain.line(x + int(xx), y + int(yy),
				  x + int(x2), y + int(y2),
				  LColor::mathline);
			break;
		}	 
		case 2: 
		case 4:
		{
			int xp[32], yp[32];
			n = int(d[i++]);
			for (j = 0; j < n; ++j) {
				xx = d[i++]; yy = d[i++];
//	     lyxerr << " " << xx << " " << yy << " ";
				if (code == 4) 
					sqmt.transf(xx, yy, xx, yy);
				else
					mt.transf(xx, yy, xx, yy);
				xp[j] = x + int(xx);
				yp[j] = y + int(yy);
				//  lyxerr << "P[" << j " " << xx << " " << yy << " " << x << " " << y << "]";
			}
			pain.lines(xp, yp, n, LColor::mathline);
		}
		}
	} while (code);
}
#else
void mathed_draw_deco(Window win, int x, int y, int w, int h, int code)
{
   Matriz mt, sqmt;
   XPoint p[32];
   float xx, yy, x2, y2;
   int i= 0, j, n;
   
   j = search_deco(code);   
   if (j < 0) return;
   
   int r = math_deco_table[j].angle;
   float * d = math_deco_table[j].data;
   
   if (h > 70 && (math_deco_table[j].code == int('(') || math_deco_table[j].code == int(')')))
      d = parenthHigh;
    
   mt.rota(r);
   mt.escala(w, h);
   
   n = (w < h) ? w: h;
   sqmt.rota(r);
   sqmt.escala(n, n);
   if (r> 0 && r< 3) y += h;   
   if (r>= 2) x += w;   
   do {
      code = int(d[i++]);
      switch (code) {
       case 0: break;
       case 1: 
       case 3:
       {
	  xx = d[i++]; yy = d[i++];
	  x2 = d[i++]; y2 = d[i++];
	  if (code == 3) 
	    sqmt.transf(xx, yy, xx, yy);
	  else
	    mt.transf(xx, yy, xx, yy);
	  mt.transf(x2, y2, x2, y2);
	  XDrawLine(fl_display, win, mathGC, x + int(xx), y + int(yy),
		    x + int(x2), y + int(y2));
	  XFlush(fl_display);
	  break;
       }	 
       case 2: 
       case 4:
       {
	  n = int(d[i++]);
	  for (j = 0; j < n; ++j) {
	     xx = d[i++]; yy = d[i++];
//	     lyxerr << " " << xx << " " << yy << " ";
	     if (code == 4) 
	       sqmt.transf(xx, yy, xx, yy);
	     else
	       mt.transf(xx, yy, xx, yy);
	     p[j].x = x + int(xx);
	     p[j].y = y + int(yy);
	     //  lyxerr << "P[" << j " " << xx << " " << yy << " " << x << " " << y << "]";
	  }
	  XDrawLines(fl_display, win, mathLineGC, p, n, CoordModeOrigin);
	  XFlush(fl_display);
       }
      }
   } while (code);
}
#endif


#ifdef USE_PAINTER
void
MathDelimInset::draw(Painter & pain, int x, int y)
{ 
	xo = x;  yo = y; 
	MathParInset::draw(pain, x + dw + 2, y - dh); 
	
	if (left == '.') {
		pain.line(x + 4, yo - ascent,
			  x + 4, yo + descent,
			  LColor::mathcursor);
	} else
		mathed_draw_deco(pain, x, y - ascent, dw, Height(), left);
	x += Width()-dw-2;
	if (right == '.') {
		pain.line(x + 4, yo - ascent,
			  x + 4, yo + descent,
			  LColor::mathcursor);
	} else
		mathed_draw_deco(pain, x, y-ascent, dw, Height(), right);
}
#else
void
MathDelimInset::Draw(int x, int y)
{ 
   xo = x;  yo = y; 
   MathParInset::Draw(x+dw+2, y-dh); 
   //int h= Height(), hg= descent-1;  

   if (left == '.') {
     XDrawLine(fl_display, pm, cursorGC, x+4, yo-ascent, x+4, yo+descent);
     XFlush(fl_display);
   }
   else
     mathed_draw_deco(pm, x, y-ascent, dw, Height(), left);
   x += Width()-dw-2;
   if (right == '.') {
     XDrawLine(fl_display, pm, cursorGC, x+4, yo-ascent, x+4, yo+descent);
     XFlush(fl_display);
   }
   else
     mathed_draw_deco(pm, x, y-ascent, dw, Height(), right);
}
#endif


void
MathDelimInset::Metrics()
{
   MathParInset::Metrics();
   int d;
   
   mathed_char_height(LM_TC_CONST, size, 'I', d, dh);
   dh /= 2;
   ascent += 2 + dh;
   descent += 2 - dh;
   dw = Height()/5;
   if (dw > 15) dw = 15;
   if (dw<6) dw = 6;
   width += 2*dw+4;
}


#ifdef USE_PAINTER
void
MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
   MathParInset::draw(pain, x + (width - dw) / 2, y);
   mathed_draw_deco(pain, x, y + dy, width, dh, deco);
}
#else
void
MathDecorationInset::Draw(int x, int y)
{ 
   MathParInset::Draw(x+(width-dw)/2, y);
   mathed_draw_deco(pm, x, y+dy, width, dh, deco);
}
#endif


void
MathDecorationInset::Metrics()
{
   int h = 2*mathed_char_height(LM_TC_VAR, size, 'I', ascent, descent);  
   MathParInset::Metrics();
   int w = Width()+4;
   if (w<16) w = 16;
   dh = w/5;
   if (dh>h) dh = h;

   if (upper) {
      ascent += dh+2;
      dy = -ascent;
   } else {
      dy = descent+2;
      descent += dh+4;
   }
   dw = width;
   width = w;
}


#ifdef USE_PAINTER
void
MathAccentInset::draw(Painter & pain, int x, int y)
{
    int dw = width - 2;

    if (inset) {
	inset->draw(pain, x, y);
    } else {
	drawStr(pain, fn, size, x, y, &c, 1);
    }
    x += (code == LM_not) ? (width-dw) / 2 : 2;
    mathed_draw_deco(pain, x, y - dy, dw, dh, code);
}
#else
void
MathAccentInset::Draw(int x, int y)
{
    int dw = width-2;
/*    char s[8];
    mathed_set_font(fn, size);
    if (MathIsBinary(fn)) {
	s[0] = s[2] = ' '; 
	s[1] = (char)c;
	ns = 3;
	dw = mathed_char_width(fn, size, c);
    } else
      s[0] = (char)c;
*/
    if (inset) {
	inset->Draw(x, y);
    } else {
	drawStr(fn, size, x, y, &c, 1);
	XFlush(fl_display);
    }
    x += (code == LM_not) ? (width-dw)/2: 2;
    mathed_draw_deco(pm, x, y-dy, dw, dh, code);
}
#endif


void
MathAccentInset::Metrics()
{
    
    if (inset) {
	inset->Metrics();
	ascent = inset->Ascent();
	descent = inset->Descent();
	width = inset->Width();
	dh = ascent;
    } else {
	mathed_char_height(fn, size, c, ascent, descent);
	width = mathed_char_width(fn, size, c);
	dh = (width-2)/2; 
    }
    if (code == LM_not) {
	ascent += dh;
	descent += dh;
	dh = Height();
    } else 
      ascent += dh+2;
	    
    dy = ascent;
//    if (MathIsBinary(fn))
//	width += 2*mathed_char_width(fn, size, ' ');    
}


#ifdef USE_PAINTER
void
MathDotsInset::draw(Painter & pain, int x, int y)
{
   mathed_draw_deco(pain, x + 2, y - dh, width - 2, ascent, code);
   if (code == LM_vdots || code == LM_ddots) ++x; 
   if (code != LM_vdots) --y;
   mathed_draw_deco(pain, x + 2, y - dh, width - 2, ascent, code);
}
#else
void
MathDotsInset::Draw(int x, int y)
{
   mathed_draw_deco(pm, x + 2, y - dh, width - 2, ascent, code);
   if (code == LM_vdots || code == LM_ddots) ++x; 
   if (code!= LM_vdots) --y;
   mathed_draw_deco(pm, x + 2, y - dh, width - 2, ascent, code);
}
#endif


void
MathDotsInset::Metrics()
{
   mathed_char_height(LM_TC_VAR, size, 'M', ascent, descent);
   width = mathed_char_width(LM_TC_VAR, size, 'M');   
   switch (code) {
    case LM_ldots: dh = 0; break;
    case LM_cdots: dh = ascent/2; break;
    case LM_vdots: width /= 2;
    case LM_ddots: dh = ascent; break;
   }
} 

