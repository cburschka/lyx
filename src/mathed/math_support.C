#include <config.h>

#include <map>

#include "math_support.h"
#include "lyxfont.h"
#include "FontLoader.h"
#include "font.h"
#include "math_defs.h"
#include "math_inset.h"
#include "math_parser.h"
#include "Painter.h"
#include "debug.h"

using std::map;
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
	Matrix(int, double, double);
	///
	void transform(double &, double &);
private:
	///
	double m_[2][2];
};


Matrix::Matrix(int code, double x, double y)
{
	double const cs = (code & 1) ? 0 : (1 - code);
	double const sn = (code & 1) ? (2 - code) : 0;
	m_[0][0] =  cs * x;
	m_[0][1] =  sn * x;
	m_[1][0] = -sn * y;
	m_[1][1] =  cs * y;
}


void Matrix::transform(double & x, double & y)
{
	double xx = m_[0][0] * x + m_[0][1] * y;
	double yy = m_[1][0] * x + m_[1][1] * y;
	x = xx;
	y = yy;
}


namespace {

LyXFont * MathFonts = 0;
bool font_available[LM_FONT_END];
bool font_available_initialized[LM_FONT_END];

enum MathFont {
	FONT_IT,
	FONT_SYMBOL,
	FONT_SYMBOLI,
	FONT_BF,
	FONT_TT,
	FONT_RM,
	FONT_SF,
	FONT_CMR,
	FONT_CMSY,
	FONT_CMM,
	FONT_CMEX,
	FONT_MSA,
	FONT_MSB,
	FONT_EUFRAK,
	FONT_FAKEBB,
	FONT_FAKECAL,
	FONT_FAKEFRAK,
	FONT_NUM
};

void mathed_init_fonts()
{
	MathFonts = new LyXFont[FONT_NUM];

	MathFonts[FONT_IT].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[FONT_SYMBOL].setFamily(LyXFont::SYMBOL_FAMILY);

	MathFonts[FONT_SYMBOLI].setFamily(LyXFont::SYMBOL_FAMILY);
	MathFonts[FONT_SYMBOLI].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[FONT_BF].setSeries(LyXFont::BOLD_SERIES);

	MathFonts[FONT_TT].setFamily(LyXFont::TYPEWRITER_FAMILY);
	MathFonts[FONT_RM].setFamily(LyXFont::ROMAN_FAMILY);
	MathFonts[FONT_SF].setFamily(LyXFont::SANS_FAMILY);

	MathFonts[FONT_CMR].setFamily(LyXFont::CMR_FAMILY);
	MathFonts[FONT_CMSY].setFamily(LyXFont::CMSY_FAMILY);
	MathFonts[FONT_CMM].setFamily(LyXFont::CMM_FAMILY);
	MathFonts[FONT_CMEX].setFamily(LyXFont::CMEX_FAMILY);
	MathFonts[FONT_MSA].setFamily(LyXFont::MSA_FAMILY);
	MathFonts[FONT_MSB].setFamily(LyXFont::MSB_FAMILY);
	MathFonts[FONT_EUFRAK].setFamily(LyXFont::EUFRAK_FAMILY);

	MathFonts[FONT_FAKEBB].setFamily(LyXFont::TYPEWRITER_FAMILY);
	MathFonts[FONT_FAKEBB].setSeries(LyXFont::BOLD_SERIES);

	MathFonts[FONT_FAKECAL].setFamily(LyXFont::SANS_FAMILY);
	MathFonts[FONT_FAKECAL].setShape(LyXFont::ITALIC_SHAPE);

	MathFonts[FONT_FAKEFRAK].setFamily(LyXFont::SANS_FAMILY);
	MathFonts[FONT_FAKEFRAK].setSeries(LyXFont::BOLD_SERIES);

	for (int i = 0; i < LM_FONT_END; ++i)
		font_available_initialized[i] = false;
}


LyXFont const & whichFontBaseIntern(MathTextCodes type)
{
	if (!MathFonts)
		mathed_init_fonts();

	switch (type) {
	case LM_TC_SYMB:	
	case LM_TC_BOLDSYMB:	
		return MathFonts[FONT_SYMBOLI];

	case LM_TC_VAR:
	case LM_TC_IT:
		return MathFonts[FONT_IT];

	case LM_TC_BF:
		return MathFonts[FONT_BF];

	case LM_TC_BB:
		return MathFonts[FONT_MSB];

	case LM_TC_CAL:
		return MathFonts[FONT_CMSY];

	case LM_TC_TT:
		return MathFonts[FONT_TT];

	case LM_TC_BOX:
	case LM_TC_TEXTRM:
	case LM_TC_CONST:
	case LM_TC_TEX:
	case LM_TC_RM:
		return MathFonts[FONT_RM];

	case LM_TC_SF:
		return MathFonts[FONT_SF];

	case LM_TC_CMR:
		return MathFonts[FONT_CMR];

	case LM_TC_CMSY:
		return MathFonts[FONT_CMSY];

	case LM_TC_CMM:
		return MathFonts[FONT_CMM];

	case LM_TC_CMEX:
		return MathFonts[FONT_CMEX];

	case LM_TC_MSA:
		return MathFonts[FONT_MSA];

	case LM_TC_MSB:
		return MathFonts[FONT_MSB];

	case LM_TC_EUFRAK:
		return MathFonts[FONT_EUFRAK];

	default:
		break;
	}
	return MathFonts[1];
}

LyXFont const & whichFontBase(MathTextCodes type)
{
	if (!MathFonts)
		mathed_init_fonts();

	switch (type) {
	case LM_TC_BB:
		if (math_font_available(LM_TC_MSB))
			return MathFonts[FONT_MSB];
		else
			return MathFonts[FONT_FAKEBB];

	case LM_TC_CAL:
		if (math_font_available(LM_TC_CMSY))
			return MathFonts[FONT_CMSY];
		else
			return MathFonts[FONT_FAKECAL];

	case LM_TC_EUFRAK:
		if (math_font_available(LM_TC_EUFRAK))
			return MathFonts[FONT_EUFRAK];
		else
			return MathFonts[FONT_FAKEFRAK];

	default:
		break;
	}
	return whichFontBaseIntern(type);
}


LyXFont whichFont(MathTextCodes type, MathMetricsInfo const & size)
{
	LyXFont f = whichFontBase(type);
	// use actual size
	f.setSize(size.font.size());

	switch (size.style) {
	case LM_ST_DISPLAY:
		if (type == LM_TC_BOLDSYMB || type == LM_TC_CMEX) {
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
		lyxerr << "Math Error: wrong font size: " << size.style << endl;
		break;
	}

	if (type != LM_TC_TEXTRM && type != LM_TC_BOX)
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
		font_available[type] = fontloader.available(whichFontBaseIntern(type));
	}
	return font_available[type];
}


namespace {

/*
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4= square polyline
 */


double const parenthHigh[] = {
	2, 13,
	0.9840, 0.0014, 0.7143, 0.0323, 0.4603, 0.0772,
	0.2540, 0.1278, 0.1746, 0.1966, 0.0952, 0.3300,
	0.0950, 0.5000, 0.0952, 0.6700, 0.1746, 0.8034,
	0.2540, 0.8722, 0.4603, 0.9228, 0.7143, 0.9677,
	0.9840, 0.9986,
	0
};


double const parenth[] = {
	2, 13,
	0.9930, 0.0071, 0.7324, 0.0578, 0.5141, 0.1126,
	0.3380, 0.1714, 0.2183, 0.2333, 0.0634, 0.3621,
	0.0141, 0.5000, 0.0563, 0.6369, 0.2113, 0.7647,
	0.3310, 0.8276, 0.5070, 0.8864, 0.7254, 0.9412,
	0.9930, 0.9919,
	0
};


double const brace[] = {
	2, 21,
	0.9492, 0.0020, 0.9379, 0.0020, 0.7458, 0.0243,
	0.5819, 0.0527, 0.4859, 0.0892, 0.4463, 0.1278,
	0.4463, 0.3732, 0.4011, 0.4199, 0.2712, 0.4615,
	0.0734, 0.4919, 0.0113, 0.5000, 0.0734, 0.5081,
	0.2712, 0.5385, 0.4011, 0.5801, 0.4463, 0.6268,
	0.4463, 0.8722, 0.4859, 0.9108, 0.5819, 0.9473,
	0.7458, 0.9757, 0.9379, 0.9980, 0.9492, 0.9980,
	0
};


double const arrow[] = {
	4, 7,
	0.0150, 0.7500, 0.2000, 0.6000, 0.3500, 0.3500,
	0.5000, 0.0500, 0.6500, 0.3500, 0.8000, 0.6000,
	0.9500, 0.7500,
	3, 0.5000, 0.1500, 0.5000, 0.9500,
	0
};


double const Arrow[] = {
	4, 7,
	0.0150, 0.7500, 0.2000, 0.6000, 0.3500, 0.3500,
	0.5000, 0.0500, 0.6500, 0.3500, 0.8000, 0.6000,
	0.9500, 0.7500,
	3, 0.3500, 0.5000, 0.3500, 0.9500,
	3, 0.6500, 0.5000, 0.6500, 0.9500,
	0
};


double const udarrow[] = {
	2, 3,
	0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
	2, 3,
	0.015, 0.75,  0.5, 0.95, 0.95, 0.75,
	1, 0.5, 0.2,  0.5, 0.8,
	0
};


double const Udarrow[] = {
	2, 3,
	0.015, 0.25,  0.5, 0.05, 0.95, 0.25,
	2, 3,
	0.015, 0.75,  0.5, 0.95, 0.95, 0.75,
	1, 0.35, 0.2, 0.35, 0.8,
	1, 0.65, 0.2, 0.65, 0.8,
	0
};


double const brack[] = {
	2, 4,
	0.95, 0.05,  0.05, 0.05,  0.05, 0.95,  0.95, 0.95,
	0
};


double const corner[] = {
	2, 3,
	0.95, 0.05,  0.05, 0.05,  0.05, 0.95,
	0
};


double const angle[] = {
	2, 3,
	1, 0,  0.05, 0.5,  1, 1,
	0
};


double const slash[] = {
	1, 0.95, 0.05, 0.05, 0.95,
	0
};


double const hline[] = {
	1, 0.00, 0.5, 1.0, 0.5,
	0
};


double const hline2[] = {
	1, 0.1, 0.5,  0.3, 0.5,
	1, 0.7, 0.5,  0.9, 0.5,
	0
};


double const hline3[] = {
	1, 0.1,   0,  0.15,  0,
	1, 0.475, 0,  0.525, 0,
	1, 0.85,  0,  0.9,   0,
	0
};


double const dline3[] = {
	1, 0.1,   0.1,   0.15,  0.15,
	1, 0.475, 0.475, 0.525, 0.525,
	1, 0.85,  0.85,  0.9,   0.9,
	0
};


double const hlinesmall[] = {
	1, 0.4, 0.5, 0.6, 0.5,
	0
};


double const vert[] = {
	1, 0.5, 0.05,  0.5, 0.95,
	0
};


double const  Vert[] = {
	1, 0.3, 0.05,  0.3, 0.95,
	1, 0.7, 0.05,  0.7, 0.95,
	0
};


double const tilde[] = {
	2, 4,
	0.05, 0.8,  0.25, 0.2,  0.75, 0.8,  0.95, 0.2,
	0
};


struct deco_struct {
	double const * data;
	int angle;
};

struct named_deco_struct {
	char const * name;
	double const * data;
	int angle;
};

named_deco_struct deco_table[] = {
	// Decorations
	{"widehat",        angle,      3 },
	{"widetilde",      tilde,      0 },
	{"underline",      hline,      0 },
	{"overline",       hline,      0 },
	{"underbrace",     brace,      1 },
	{"overbrace",      brace,      3 },
	{"overleftarrow",  arrow,      1 },
	{"overrightarrow", arrow,      3 },
	
	// Delimiters
	{"(",              parenth,    0 },
	{")",              parenth,    2 },
	{"{",              brace,      0 },
	{"}",              brace,      2 },
	{"[",              brack,      0 },
	{"]",              brack,      2 },
	{"|",              vert,       0 },
	{"/",              slash,      0 },
	{"Vert",           Vert,       0 },
	{"'",              slash,      1 },
	{"backslash",      slash,      1 },
	{"langle",         angle,      0 },
	{"lceil",          corner,     0 },
	{"lfloor",         corner,     1 },
	{"rangle",         angle,      2 },
	{"rceil",          corner,     3 },
	{"rfloor",         corner,     2 },
	{"downarrow",      arrow,      2 },
	{"Downarrow",      Arrow,      2 },
	{"uparrow",        arrow,      0 },
	{"Uparrow",        Arrow,      0 },
	{"updownarrow",    udarrow,    0 },
	{"Updownarrow",    Udarrow,    0 },	
	
	// Accents
	{"ddot",           hline2,     0 },
	{"hat",            angle,      3 },
	{"grave",          slash,      1 },
	{"acute",          slash,      0 },
	{"tilde",          tilde,      0 },
	{"bar",            hline,      0 },
	{"dot",            hlinesmall, 0 },
	{"check",          angle,      1 },
	{"breve",          parenth,    1 },
	{"vec",            arrow,      3 },
	{"not",            slash,      0 },
	
	// Dots
	{"ldots",          hline3,     0 },
	{"cdots",          hline3,     0 },
	{"vdots",          hline3,     1 },
	{"ddots",          dline3,     0 }
};


map<string, deco_struct> deco_list;

// sort the table on startup
struct init_deco_table {
	init_deco_table() {
		unsigned const n = sizeof(deco_table) / sizeof(deco_table[0]);
		for (named_deco_struct * p = deco_table; p != deco_table + n; ++p) {
			deco_struct d;
			d.data  = p->data;
			d.angle = p->angle;
			deco_list[p->name]= d;
		}
	}
};

static init_deco_table dummy;


deco_struct const * search_deco(string const & name)
{
	map<string, deco_struct>::const_iterator p = deco_list.find(name);
	return (p == deco_list.end()) ? 0 : &(p->second);
}


} // namespace anon


void mathed_char_dim(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c, int & asc, int & des, int & wid)
{
	LyXFont const font = whichFont(type, size);
	des = lyxfont::descent(c, font);
	asc = lyxfont::ascent(c, font);
	wid = mathed_char_width(type, size, c);
}


int mathed_char_height(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c, int & asc, int & des)
{
	LyXFont const font = whichFont(type, size);
	des = lyxfont::descent(c, font);
	asc = lyxfont::ascent(c, font);
	return asc + des;
}


int mathed_char_height(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c)
{
	int asc;
	int des;
	return mathed_char_height(type, size, c, asc, des);
}


int mathed_char_ascent(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	return lyxfont::ascent(c, font);
}


int mathed_char_descent(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	return lyxfont::descent(c, font);
}


int mathed_char_width(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c)
{
	LyXFont const font = whichFont(type, size);
	if (isBinaryOp(c, type))
		return lyxfont::width(c, font) + 2 * lyxfont::width(' ', font);
	else
		return lyxfont::width(c, font);
}


void mathed_string_dim(MathTextCodes type, MathMetricsInfo const & size,
	string const & s, int & asc, int & des, int & wid)
{
	mathed_string_height(type, size, s, asc, des);
	wid = mathed_string_width(type, size, s);
}


int mathed_string_height(MathTextCodes type, MathMetricsInfo const & size,
	string const & s, int & asc, int & des)
{
	LyXFont const font = whichFont(type, size);
	asc = des = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		des = max(des, lyxfont::descent(*it, font));
		asc = max(asc, lyxfont::ascent(*it, font));
	}
	return asc + des;
}


int mathed_string_width(MathTextCodes type, MathMetricsInfo const & size,
	string const & s)
{
	return lyxfont::width(s, whichFont(type, size));
}


int mathed_string_ascent(MathTextCodes type, MathMetricsInfo const & size,
	string const & s)
{
	LyXFont const font = whichFont(type, size);
	int asc = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it)
		asc = max(asc, lyxfont::ascent(*it, font));
	return asc;
}


int mathed_string_descent(MathTextCodes type, MathMetricsInfo const & size,
	string const & s)
{
	LyXFont const font = whichFont(type, size);
	int des = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it)
		des = max(des, lyxfont::descent(*it, font));
	return des;
}



void mathed_draw_deco(Painter & pain, int x, int y, int w, int h,
	const string & name)
{
	if (name == ".") {
		pain.line(x + w/2, y, x + w/2, y + h,
			  LColor::mathcursor, Painter::line_onoffdash);
		return;
	}	
	
	deco_struct const * mds = search_deco(name);
	if (!mds) {
		lyxerr << "Deco was not found. Programming error?\n";
		lyxerr << "name: '" << name << "'\n";
		return;
	}
	
	int const n = (w < h) ? w : h;
	int const r = mds->angle;
	double const * d = mds->data;
	
	if (h > 70 && (name == "(" || name == ")"))
		d = parenthHigh;
	
	Matrix mt(r, w, h);
	Matrix sqmt(r, n, n);

	if (r > 0 && r < 3)
		y += h;

	if (r >= 2)
		x += w;

	for (int i = 0; d[i]; ) {
		int code = int(d[i++]);
		if (code & 1) {  // code == 1 || code == 3
			double xx = d[i++];
			double yy = d[i++];
			double x2 = d[i++];
			double y2 = d[i++];
			if (code == 3)
				sqmt.transform(xx, yy);
			else
				mt.transform(xx, yy);
			mt.transform(x2, y2);
			pain.line(x + int(xx), y + int(yy), x + int(x2), y + int(y2),
					LColor::math);
		}	else {
			int xp[32];
			int yp[32];
			int const n = int(d[i++]);
			for (int j = 0; j < n; ++j) {
				double xx = d[i++];
				double yy = d[i++];
//	     lyxerr << " " << xx << " " << yy << " ";
				if (code == 4)
					sqmt.transform(xx, yy);
				else
					mt.transform(xx, yy);
				xp[j] = x + int(xx);
				yp[j] = y + int(yy);
				//  lyxerr << "P[" << j " " << xx << " " << yy << " " << x << " " << y << "]";
			}
			pain.lines(xp, yp, n, LColor::math);
		}
	}
}


// In the future maybe we use a better fonts renderer
void drawStr(Painter & pain, MathTextCodes type, MathMetricsInfo const & siz,
	int x, int y, string const & s)
{
	pain.text(x, y, s, whichFont(type, siz));
}


void drawChar(Painter & pain, MathTextCodes type, MathMetricsInfo const & siz,
	int x, int y, char c)
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
void smallerStyleScript(MathMetricsInfo & st)
{
	switch (st.style) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:    st.style = LM_ST_SCRIPT; break;
		default:            st.style = LM_ST_SCRIPTSCRIPT;
	}
}


// decrease math size for fractions
void smallerStyleFrac(MathMetricsInfo & st)
{
	switch (st.style) {
		case LM_ST_DISPLAY: st.style = LM_ST_TEXT; break;
		case LM_ST_TEXT:    st.style = LM_ST_SCRIPT; break;
		default:            st.style = LM_ST_SCRIPTSCRIPT;
	}
}


void math_font_max_dim(MathTextCodes code, MathMetricsInfo const & siz,
	int & asc, int & des)
{
	LyXFont font = whichFont(code, siz);
	asc = lyxfont::maxAscent(font);
	des = lyxfont::maxDescent(font);
}


char const * latex_mathspace[] = {
	"!", ",", ":", ";", "quad", "qquad"
};


char const * math_font_name(MathTextCodes code)
{
	static char const * theFontNames[] = {
		"mathrm",
		"mathcal",
		"mathfrak",
		"mathbf",
		"mathbb",
		"mathsf",
		"mathtt",
		"mathit",
		"textrm"
	};

	if (code >= LM_TC_RM && code <= LM_TC_TEXTRM) 
		return theFontNames[code - LM_TC_RM];
	return 0;
}
