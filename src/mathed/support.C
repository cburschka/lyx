#include <config.h>

#include <algorithm>

#include "mathed/support.h"
#include "lyxfont.h"
#include "font.h"
#include "math_defs.h"
#include "Painter.h"
#include "matriz.h"
#include "symbol_def.h"

extern LyXFont WhichFont(short type, int size);

char const * math_font_name[] = {
   "mathrm",
   "mathcal",
   "mathbf",
   "mathsf",
   "mathtt",
   "mathit",
   "textrm"
};


char const * latex_mathspace[] = {
   "!", ",", ":", ";", "quad", "qquad"
};

/* 
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4= square polyline
 */


static
float parenthHigh[] = {
    2.0, 13.0, 0.9840, 0.0014, 0.7143, 0.0323, 0.4603, 0.0772, 0.2540, 
    0.1278, 0.1746, 0.1966, 0.0952, 0.3300, 0.0950, 0.5000, 0.0952, 0.6700, 
    0.1746, 0.8034, 0.2540, 0.8722, 0.4603, 0.9228, 0.7143, 0.9677, 0.9840, 
    0.9986, 0.0 
};


static float parenth[] = {
  2.0, 13.0,
  0.9930, 0.0071,  0.7324, 0.0578,  0.5141, 0.1126,  0.3380, 0.1714,
  0.2183, 0.2333,  0.0634, 0.3621,  0.0141, 0.5000,  0.0563, 0.6369,
  0.2113, 0.7647,  0.3310, 0.8276,  0.5070, 0.8864,  0.7254, 0.9412,
  0.9930, 0.9919,
  0.0   
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


static
math_deco_struct math_deco_table[] = {   

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
  { LM_ddots, &dline3[0], 0 }
};


struct math_deco_compare {
	/// for use by sort and lower_bound
	inline
	int operator()(math_deco_struct const & a,
		       math_deco_struct const & b) const {
		return a.code < b.code;
	}
};


static
int const math_deco_table_size = sizeof(math_deco_table) /sizeof(math_deco_struct);

class init_deco_table {
public:
	init_deco_table() {
		if (!init) {
			sort(math_deco_table,
			     math_deco_table + math_deco_table_size,
			     math_deco_compare());
			init_deco_table::init = true;
		}
	}
private:
	static bool init;
};

bool init_deco_table::init = false;
static init_deco_table idt;


int mathed_char_height(short type, int size, byte c, int & asc, int & des)
{
   LyXFont font = WhichFont(type, size);
   des = lyxfont::descent(c, font);
   asc = lyxfont::ascent(c, font);
   return asc + des;
}


int mathed_char_width(short type, int size, byte c)
{
	if (MathIsBinary(type)) {
		string s;
		s += c;
		return mathed_string_width(type, size, s);
	}
	else
    return lyxfont::width(c, WhichFont(type, size));
}


int mathed_string_height(short type, int size, string const & s,
			 int & asc, int & des)
{
	LyXFont font = WhichFont(type, size);
	asc = des = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		des = max(des, lyxfont::descent(*it, font));
		asc = max(asc, lyxfont::ascent(*it, font));
	}
	return asc + des;
}


int mathed_string_width(short type, int size, string const & s)
{
	string st;
	if (MathIsBinary(type))
		for (string::const_iterator it = s.begin();
		     it != s.end(); ++it) {
			st += ' ';
			st += *it;
			st += ' ';
		}
	else
		st = s;

	LyXFont const f = WhichFont(type, size);
	return lyxfont::width(st, f);
}


LyXFont mathed_get_font(short type, int size)
{
	LyXFont f = WhichFont(type, size);
	if (type == LM_TC_TEX) {
		f.setLatex(LyXFont::ON);
	}
	return f;
}


void mathed_draw_deco(Painter & pain, int x, int y, int w, int h, int code)
{
	Matriz mt;
	Matriz sqmt;
	float xx;
	float yy;
	float x2;
	float y2;
	int i = 0;

#if USE_EXCEPTIONS
	math_deco_struct mds;
	try {
		mds = search_deco(code);
	}
	catch (deco_not_found) {
		// Should this ever happen?
		lyxerr << "Deco was not found. Programming error?" << endl;
		return;
	}
   
	int r = mds.angle;
	float * d = mds.data;
	
	if (h > 70 && (mds.code == int('(')
		       || mds.code == int(')')))
		d = parenthHigh;
#else
	math_deco_struct const * mds = search_deco(code);
	if (!mds) {
		// Should this ever happen?
		lyxerr << "Deco was not found. Programming error?" << endl;
		return;
	}
	
   
	int r = mds->angle;
	float * d = mds->data;
	
	if (h > 70 && (mds->code == int('(')
		       || mds->code == int(')')))
		d = parenthHigh;
#endif
	
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
			for (int j = 0; j < n; ++j) {
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


#define USE_EXCEPTIONS 0
#if USE_EXCEPTIONS
struct deco_not_found {};


math_deco_struct const & search_deco(int code)
{
	math_deco_struct * res =
		lower_bound(math_deco_table,
			    math_deco_table + math_deco_table_size,
			    code, math_deco_compare());
	if (res != math_deco_table + math_deco_table_size &&
	    res->code == code)
		return *res;
	throw deco_not_found();
}

#else


math_deco_struct const * search_deco(int code)
{
	math_deco_struct search_elem = { code, 0, 0 };
	
	math_deco_struct * res =
		lower_bound(math_deco_table,
			    math_deco_table + math_deco_table_size,
			    search_elem, math_deco_compare());
	if (res != math_deco_table + math_deco_table_size &&
	    res->code == code)
		return res;
	return 0;
}
#endif
