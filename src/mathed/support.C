#include <config.h>

#include <algorithm>

#include "mathed/support.h"
#include "lyxfont.h"
#include "FontLoader.h"
#include "font.h"
#include "math_defs.h"
#include "math_parser.h"
#include "Painter.h"
#include "debug.h"

using std::sort;
using std::lower_bound;
using std::endl;
using std::max;


bool isBinaryOp(char c, MathTextCodes type)
{
	return type < LM_TC_SYMB && strchr("+-<>=/*", c); 
}


///
class Matrix {
public:
	///
	typedef float matriz_data[2][2];
	///
	Matrix();
	///
	void rotate(int);
	///
	void escalate(float, float);
	///
	void transform(float, float, float &, float &);
private:
	///
	matriz_data m_;
	///
	void multiply(matriz_data & a);
};

Matrix::Matrix()
{
	m_[0][0] = 1;
	m_[0][1] = 0;
	m_[1][0] = 0;
	m_[1][1] = 1;
}

void Matrix::rotate(int code)
{
	matriz_data r;
	r[0][0] = 1;
	r[0][1] = 0;
	r[1][0] = 0;
	r[1][1] = 1;
	float const cs = (code & 1) ? 0 : (1 - code);
	float const sn = (code & 1) ? (2 - code) : 0;
	r[0][0] = cs;
	r[0][1] = sn;
	r[1][0] = -r[0][1];
	r[1][1] = r[0][0];
	multiply(r);
}

void Matrix::escalate(float x, float y)
{
	matriz_data s;
	s[0][0] = x;
	s[0][1] = 0;
	s[1][0] = 0;
	s[1][1] = y;
	multiply(s);
}

void Matrix::multiply(matriz_data & a)
{
	matriz_data c;
	c[0][0] = a[0][0] * m_[0][0] + a[0][1] * m_[1][0];
	c[1][0] = a[1][0] * m_[0][0] + a[1][1] * m_[1][0];
	c[0][1] = a[0][0] * m_[0][1] + a[0][1] * m_[1][1];
	c[1][1] = a[1][0] * m_[0][1] + a[1][1] * m_[1][1];
	m_[0][0] = c[0][0];	
	m_[0][1] = c[0][1];	
	m_[1][0] = c[1][0];	
	m_[1][1] = c[1][1];	
}

void Matrix::transform(float xp, float yp, float & x, float & y)
{
	x = m_[0][0] * xp + m_[0][1] * yp;
	y = m_[1][0] * xp + m_[1][1] * yp;
}


namespace {

LyXFont * MathFonts = 0;
bool font_available[LM_FONT_END];
bool font_available_initialized[LM_FONT_END];

void mathed_init_fonts()
{
	MathFonts = new LyXFont[13]; //DEC cxx cannot initialize all fonts
	//at once (JMarc) rc

	for (int i = 0 ; i < 13 ; ++i) {
		MathFonts[i] = LyXFont(LyXFont::ALL_SANE);
	}

	MathFonts[0].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[1].setFamily(LyXFont::SYMBOL_FAMILY);

	MathFonts[2].setFamily(LyXFont::SYMBOL_FAMILY);
	MathFonts[2].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[3].setSeries(LyXFont::BOLD_SERIES);

	MathFonts[4].setFamily(LyXFont::SANS_FAMILY);
	MathFonts[4].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[5].setFamily(LyXFont::TYPEWRITER_FAMILY);

	MathFonts[6].setFamily(LyXFont::ROMAN_FAMILY);

	MathFonts[7].setFamily(LyXFont::SANS_FAMILY);

	MathFonts[8].setFamily(LyXFont::CMSY_FAMILY);
	MathFonts[9].setFamily(LyXFont::CMM_FAMILY);
	MathFonts[10].setFamily(LyXFont::CMEX_FAMILY);
	MathFonts[11].setFamily(LyXFont::MSA_FAMILY);
	MathFonts[12].setFamily(LyXFont::MSB_FAMILY);

	for (int i = 0; i < LM_FONT_END; ++i)
		font_available_initialized[i] = false;
}


LyXFont const & whichFontBase(MathTextCodes type)
{
	if (!MathFonts)
		mathed_init_fonts();

	switch (type) {
	case LM_TC_SYMB:	
	case LM_TC_BSYM:	
		return MathFonts[2];

	case LM_TC_VAR:
	case LM_TC_IT:
		return MathFonts[0];

	case LM_TC_BF:
		return MathFonts[3];

	case LM_TC_CAL:
		return MathFonts[4];

	case LM_TC_TT:
		return MathFonts[5];

	case LM_TC_TEXTRM:
	case LM_TC_CONST:
	case LM_TC_TEX:
	case LM_TC_RM:
		return MathFonts[6];

	case LM_TC_SF:
		return MathFonts[7];

	case LM_TC_CMSY:
		return MathFonts[8];

	case LM_TC_CMM:
		return MathFonts[9];

	case LM_TC_CMEX:
		return MathFonts[10];

	case LM_TC_MSA:
		return MathFonts[11];

	case LM_TC_MSB:
		return MathFonts[12];

	default:
		break;
	}
	return MathFonts[1];
}


LyXFont whichFont(MathTextCodes type, MathStyles size)
{
	LyXFont f = whichFontBase(type);

	switch (size) {
	case LM_ST_DISPLAY:
		if (type == LM_TC_BSYM || type == LM_TC_CMEX) {
			f.incSize();
			f.incSize();
		}
		break;

	case LM_ST_TEXT:
		break;

	case LM_ST_SCRIPT:
		f.decSize();
		f.decSize();
		break;

	case LM_ST_SCRIPTSCRIPT:
		f.decSize();
		f.decSize();
		f.decSize();
		break;

	default:
		lyxerr << "Math Error: wrong font size: " << size << endl;
		break;
	}

	if (type != LM_TC_TEXTRM)
		f.setColor(LColor::math);

	if (type == LM_TC_TEX)
		f.setColor(LColor::latex);

	return f;
}

} // namespace


bool math_font_available(MathTextCodes type)
{
	if (!font_available_initialized[type]) {
		font_available_initialized[type] = true;
		font_available[type] = 
			fontloader.available(whichFontBase(type));
	}
	return font_available[type];
}


namespace {

/* 
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4= square polyline
 */


float const parenthHigh[] = {
	2.0, 13.0,
	0.9840, 0.0014, 0.7143, 0.0323, 0.4603, 0.0772,
	0.2540, 0.1278, 0.1746, 0.1966, 0.0952, 0.3300,
	0.0950, 0.5000, 0.0952, 0.6700, 0.1746, 0.8034,
	0.2540, 0.8722, 0.4603, 0.9228, 0.7143, 0.9677,
	0.9840, 0.9986,
	0.0 
};


float const parenth[] = {
	2.0, 13.0,
	0.9930, 0.0071, 0.7324, 0.0578, 0.5141, 0.1126,
	0.3380, 0.1714, 0.2183, 0.2333, 0.0634, 0.3621,
	0.0141, 0.5000, 0.0563, 0.6369, 0.2113, 0.7647,
	0.3310, 0.8276, 0.5070, 0.8864, 0.7254, 0.9412,
	0.9930, 0.9919,
	0.0   
};


float const brace[] = {
	2.0, 21.0,
	0.9492, 0.0020, 0.9379, 0.0020, 0.7458, 0.0243,
	0.5819, 0.0527, 0.4859, 0.0892, 0.4463, 0.1278,
	0.4463, 0.3732, 0.4011, 0.4199, 0.2712, 0.4615,
	0.0734, 0.4919, 0.0113, 0.5000, 0.0734, 0.5081,
	0.2712, 0.5385, 0.4011, 0.5801, 0.4463, 0.6268,
	0.4463, 0.8722, 0.4859, 0.9108, 0.5819, 0.9473,
	0.7458, 0.9757, 0.9379, 0.9980, 0.9492, 0.9980,
	0.0
};


// Is this correct? (Lgb)
float const arrow[] = {
	4, 7,
	0.0150, 0.7500, 0.2000, 0.6000, 0.3500, 0.3500,
	0.5000, 0.0500, 0.6500, 0.3500, 0.8000, 0.6000,
	0.9500, 0.7500,
	3, 0.5000, 0.1500, 0.5000, 0.9500,
	0.0 
};


// Is this correct? (Lgb)
float const Arrow[] = {
	4, 7,
	0.0150, 0.7500, 0.2000, 0.6000, 0.3500, 0.3500,
	0.5000, 0.0500, 0.6500, 0.3500, 0.8000, 0.6000,
	0.9500, 0.7500,
	3, 0.3500, 0.5000, 0.3500, 0.9500,
	3, 0.6500, 0.5000, 0.6500, 0.9500,
	0.0
};


float const udarrow[] = {
	2, 3,
	0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
	2, 3,
	0.015, 0.75,  0.5, 0.95, 0.95, 0.75,  
	1, 0.5, 0.2,  0.5, 0.8,
	0.0 
};


float const Udarrow[] = {
	2, 3,
	0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
	2, 3,
	0.015, 0.75,  0.5, 0.95, 0.95, 0.75,  
	1, 0.35, 0.2, 0.35, 0.8,
	1, 0.65, 0.2, 0.65, 0.8,
	0.0 
};


float const brack[] = {
	2.0, 4,
	0.95, 0.05,  0.05, 0.05,  0.05, 0.95,  0.95, 0.95,
	0.0
};


float const corner[] = {
	2.0, 3,
	0.95, 0.05,  0.05, 0.05,  0.05, 0.95,
	0.0
};


float const angle[] = {
	2.0, 3,
	1, 0,  0.05, 0.5,  1, 1,
	0.0
};


float const slash[] = {
	1, 0.95, 0.05,  0.05, 0.95, 
	0.0
};


float const hline[] = {
	1, 0.05, 0.5,  0.95, 0.5, 
	0.0
};


float const hline2[] = {
   1, 0.1, 0.5,  0.3, 0.5,
   1, 0.7, 0.5,  0.9, 0.5,
   0.0
}; 


float const hline3[] = {
	1, 0.1, 0,  0.15, 0,
	1, 0.475, 0,  0.525, 0,
	1, 0.85, 0,  0.9, 0,  
	0.0
};


float const dline3[] = {
	1, 0.1, 0.1,  0.15, 0.15,
	1, 0.475, 0.475,  0.525, 0.525,
	1, 0.85, 0.85,  0.9, 0.9,
	0.0
};     


float const hlinesmall[] = {
	1, 0.4, 0.5,  0.6, 0.5, 
	0.0
};


float const vert[] = {
	1, 0.5, 0.05,  0.5, 0.95, 
	0.0
};


float const  Vert[] = {
	1, 0.3, 0.05,  0.3, 0.95, 
	1, 0.7, 0.05,  0.7, 0.95,
	0.0
};


float const tilde[] = {
	2.0, 4,
	0.05, 0.8,  0.25, 0.2,  0.75, 0.8,  0.95, 0.2,
	0.0
};


struct deco_struct {
	int code;
	float const * data;
	int angle;
};

deco_struct deco_table[] = {   
	// Decorations
	{ LM_widehat,       &angle[0],      3 },
	{ LM_widetilde,     &tilde[0],      0 },
	{ LM_underline,     &hline[0],      0 },
	{ LM_overline,      &hline[0],      0 },
	{ LM_underbrace,    &brace[0],      1 },
	{ LM_overbrace,     &brace[0],      3 },
	{ LM_overleftarrow, &arrow[0],      1 },
	{ LM_overightarrow, &arrow[0],      3 },
	                                    
	// Delimiters                       
	{ '(',              &parenth[0],    0 },
	{ ')',              &parenth[0],    2 },
	{ '{',              &brace[0],      0 },
	{ '}',              &brace[0],      2 },
	{ '[',              &brack[0],      0 },
	{ ']',              &brack[0],      2 },
	{ '|',              &vert[0],       0 },
	{ '/',              &slash[0],      0 },
	{ LM_Vert,          &Vert[0],       0 },
	{ '\\',             &slash[0],      1 },
	{ LM_langle,        &angle[0],      0 },
	{ LM_lceil,         &corner[0],     0 }, 
	{ LM_lfloor,        &corner[0],     1 },  
	{ LM_rangle,        &angle[0],      2 }, 
	{ LM_rceil,         &corner[0],     3 }, 
	{ LM_rfloor,        &corner[0],     2 },
	{ LM_downarrow,     &arrow[0],      2 },
	{ LM_Downarrow,     &Arrow[0],      2 }, 
	{ LM_uparrow,       &arrow[0],      0 },
	{ LM_Uparrow,       &Arrow[0],      0 },
	{ LM_updownarrow,   &udarrow[0],    0 },
	{ LM_Updownarrow,   &Udarrow[0],    0 },	 
	                                    
	// Accents                          
	{ LM_ddot,          &hline2[0],     0 },
	{ LM_hat,           &angle[0],      3 },
	{ LM_grave,         &slash[0],      1 },
	{ LM_acute,         &slash[0],      0 },
	{ LM_tilde,         &tilde[0],      0 },
	{ LM_bar,           &hline[0],      0 },
	{ LM_dot,           &hlinesmall[0], 0 },
	{ LM_check,         &angle[0],      1 },
	{ LM_breve,         &parenth[0],    1 },
	{ LM_vec,           &arrow[0],      3 },
	{ LM_not,           &slash[0],      0 },  
	                                    
	// Dots                             
	{ LM_ldots,         &hline3[0],     0 }, 
	{ LM_cdots,         &hline3[0],     0 },
	{ LM_vdots,         &hline3[0],     1 },
	{ LM_ddots,         &dline3[0],     0 }
};


struct deco_compare {
	/// for use by sort and lower_bound
	int operator()(deco_struct const & a, deco_struct const & b) const
	{
		return a.code < b.code;
	}
};


int const deco_table_size =
	sizeof(deco_table) / sizeof(deco_struct);


// sort the table on startup
struct init_deco_table {
	init_deco_table() {
			std::sort(deco_table,
			     deco_table + deco_table_size,
			     deco_compare());
	}
};

static init_deco_table dummy;


deco_struct const * search_deco(int code)
{
	const deco_struct search_elem = { code, 0, 0 };
	
	deco_struct const * res =
		lower_bound(deco_table, deco_table + deco_table_size, search_elem,
			deco_compare());
	if (res != deco_table + deco_table_size &&
	    res->code == code)
		return res;
	return 0;
}


} // namespace anon


void mathed_char_dim(MathTextCodes type, MathStyles size, unsigned char c,
	int & asc, int & des, int & wid)
{
	LyXFont const font = whichFont(type, size);
	des = lyxfont::descent(c, font);
	asc = lyxfont::ascent(c, font);
	wid = mathed_char_width(type, size, c);
}


int mathed_char_height(MathTextCodes type, MathStyles size, unsigned char c,
	int & asc, int & des)
{
	LyXFont const font = whichFont(type, size);
	des = lyxfont::descent(c, font);
	asc = lyxfont::ascent(c, font);
	return asc + des;
}


int mathed_char_height(MathTextCodes type, MathStyles size, unsigned char c)
{
	int asc;
	int des;
	return mathed_char_height(type, size, c, asc, des);
}


int mathed_char_ascent(MathTextCodes type, MathStyles size, unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	return lyxfont::ascent(c, font);
}


int mathed_char_descent(MathTextCodes type, MathStyles size, unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	return lyxfont::descent(c, font);
}


int mathed_char_width(MathTextCodes type, MathStyles size, unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	LyXFont const f1 = whichFont(LM_TC_TEXTRM, size);
#warning why f1 is used ?
	if (isBinaryOp(c, type))
		return lyxfont::width(c, font) + 2 * lyxfont::width(' ', f1);
	else
		return lyxfont::width(c, font);
}


void mathed_string_dim(MathTextCodes type, MathStyles size, string const & s,
			 int & asc, int & des, int & wid)
{
	mathed_string_height(type, size, s, asc, des);
	wid = mathed_string_width(type, size, s);
}


int mathed_string_height(MathTextCodes type, MathStyles size, string const & s,
			 int & asc, int & des)
{
	LyXFont const font = whichFont(type, size);
	asc = des = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		des = max(des, lyxfont::descent(*it, font));
		asc = max(asc, lyxfont::ascent(*it, font));
	}
	return asc + des;
}


int mathed_string_width(MathTextCodes type, MathStyles size, string const & s)
{
	return lyxfont::width(s, whichFont(type, size));
}


void mathed_draw_deco(Painter & pain, int x, int y, int w, int h,
	latexkeys const * l)
{
	Matrix mt;
	Matrix sqmt;
	int i = 0;
	string const & name = l->name;
	int code = (name.size() > 1) ? l->id : name[0];

	if (name == ".") {
		pain.line(x + w/2, y, x + w/2, y + h,
			  LColor::mathcursor, Painter::line_onoffdash);
		return;
	}	
	
	deco_struct const * mds = search_deco(code);
	if (!mds) {
		lyxerr << "Deco was not found. Programming error?\n";
		lyxerr << "name: '" << name << "', code: " << code << "\n";
		return;
	}
	
	int const r = mds->angle;
	float const * d = mds->data;
	
	if (h > 70 && (mds->code == int('(') || mds->code == int(')')))
		d = parenthHigh;
	
	mt.rotate(r);
	mt.escalate(w, h);
	
	int const n = (w < h) ? w : h;

	sqmt.rotate(r);
	sqmt.escalate(n, n);

	if (r > 0 && r < 3)
		y += h;

	if (r >= 2)
		x += w;   

	do {
		float xx;
		float yy;
		float x2;
		float y2;

		code = int(d[i++]);
		switch (code) {
		case 0: break;
		case 1: 
		case 3:
		{
			xx = d[i++]; yy = d[i++];
			x2 = d[i++]; y2 = d[i++];
			if (code == 3) 
				sqmt.transform(xx, yy, xx, yy);
			else
				mt.transform(xx, yy, xx, yy);
			mt.transform(x2, y2, x2, y2);
			pain.line(x + int(xx), y + int(yy),
				  x + int(x2), y + int(y2),
				  LColor::mathline);
			break;
		}	 
		case 2: 
		case 4:
		{
			int xp[32];
			int yp[32];
			int const n = int(d[i++]);
			for (int j = 0; j < n; ++j) {
				xx = d[i++]; yy = d[i++];
//	     lyxerr << " " << xx << " " << yy << " ";
				if (code == 4) 
					sqmt.transform(xx, yy, xx, yy);
				else
					mt.transform(xx, yy, xx, yy);
				xp[j] = x + int(xx);
				yp[j] = y + int(yy);
				//  lyxerr << "P[" << j " " << xx << " " << yy << " " << x << " " << y << "]";
			}
			pain.lines(xp, yp, n, LColor::mathline);
		}
		}
	} while (code);
}


// In the future maybe we use a better fonts renderer
void drawStr(Painter & pain, MathTextCodes type, MathStyles siz,
	int x, int y, string const & s)
{
	pain.text(x, y, s, whichFont(type, siz));
}


void drawChar
	(Painter & pain, MathTextCodes type, MathStyles siz, int x, int y, char c)
{
	string s;
	if (isBinaryOp(c, type))
		s += ' ';
	s += c;
	if (isBinaryOp(c, type))
		s += ' ';
	drawStr(pain, type, siz, x, y, s);
}


// decrease math size for super- and subscripts
MathStyles smallerStyleScript(MathStyles st)
{
	switch (st) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:    st = LM_ST_SCRIPT; break;
		default:            st = LM_ST_SCRIPTSCRIPT;
	}
	return st;
}


// decrease math size for fractions
MathStyles smallerStyleFrac(MathStyles st)
{
	switch (st) {
		case LM_ST_DISPLAY: st = LM_ST_TEXT; break;
		case LM_ST_TEXT:    st = LM_ST_SCRIPT; break;
		default:            st = LM_ST_SCRIPTSCRIPT;
	}
	return st;
}


void math_font_max_dim(MathTextCodes code, MathStyles siz, int & asc, int & des)
{
	LyXFont font = whichFont(code, siz);
	asc = lyxfont::maxAscent(font);
	des = lyxfont::maxDescent(font);
}


char const * latex_mathspace[] = {
	"!", ",", ":", ";", "quad", "qquad"
};
