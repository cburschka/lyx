/**
 * \file MathSupport.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathSupport.h"

#include "InsetMathFont.h"
#include "InsetMathSymbol.h"
#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"

#include "MetricsInfo.h"

#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/lyxlib.h"

#include <map>
#include <algorithm>

using namespace std;

namespace lyx {

using frontend::Painter;


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

/*
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4 = square polyline
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


double const mapsto[] = {
	2, 3,
	0.75, 0.015, 0.95, 0.5, 0.75, 0.985,
	1, 0.015, 0.475, 0.945, 0.475,
	1, 0.015, 0.015, 0.015, 0.985,
	0
};


double const lhook[] = {
	2, 3,
	0.25, 0.015, 0.05, 0.5, 0.25, 0.985,
	1, 0.015, 0.475, 0.7, 0.475,
	2, 5,
	0.7, 0.015, 0.825, 0.15, 0.985, 0.25,
	0.825, 0.35, 0.7, 0.475,
	0
};


double const rhook[] = {
	2, 3,
	0.75, 0.015, 0.95, 0.5, 0.75, 0.985,
	1, 0.3, 0.475, 0.985, 0.475,
	2, 5,
	0.3, 0.015, 0.175, 0.15, 0.05, 0.25,
	0.175, 0.35, 0.3, 0.475,
	0
};


double const LRArrow[] = {
	2, 3,
	0.25, 0.015, 0.05, 0.5, 0.25, 0.985,
	2, 3,
	0.75, 0.015, 0.95, 0.5, 0.75, 0.985,
	1, 0.2, 0.8, 0.8, 0.8,
	1, 0.2, 0.2, 0.8, 0.2,
	0
};


double const LArrow[] = {
	2, 3,
	0.25, 0.015, 0.05, 0.5, 0.25, 0.985,
	1, 0.2, 0.8, 0.985, 0.8,
	1, 0.2, 0.2, 0.985, 0.2,
	0
};


double const lharpoondown[] = {
	2, 2,
	0.015, 0.5, 0.25, 0.985,
	1, 0.02, 0.475, 0.985, 0.475,
	0
};


double const lharpoonup[] = {
	2, 2,
	0.25, 0.015, 0.015, 0.5,
	1, 0.02, 0.525, 0.985, 0.525,
	0
};


double const lrharpoons[] = {
	2, 2,
	0.25, 0.015, 0.015, 0.225,
	1, 0.02, 0.23, 0.985, 0.23,
	2, 2,
	0.75, 0.985, 0.985, 0.775,
	1, 0.02, 0.7, 0.980, 0.7,
	0
};


double const rlharpoons[] = {
	2, 2,
	0.75, 0.015, 0.985, 0.225,
	1, 0.02, 0.23, 0.985, 0.23,
	2, 2,
	0.25, 0.985, 0.015, 0.775,
	1, 0.02, 0.7, 0.980, 0.7,
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
	1, 0.5, 0.1,  0.5, 0.9,
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


double const dbrack[] = {
	2, 4,
	0.95, 0.05,  0.05, 0.05,  0.05, 0.95,  0.95, 0.95,
	2, 2,
	0.50, 0.05,  0.50, 0.95,
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


double const ddot[] = {
	1, 0.2, 0.5, 0.3, 0.5,
	1, 0.7, 0.5, 0.8, 0.5,
	0
};


double const dddot[] = {
	1, 0.1,  0.5, 0.2,  0.5,
	1, 0.45, 0.5, 0.55, 0.5,
	1, 0.8,  0.5, 0.9,  0.5,
	0
};


double const ddddot[] = {
	1, 0.1,  0.5, 0.2,  0.5,
	1, 0.45, 0.5, 0.55, 0.5,
	1, 0.8,  0.5, 0.9,  0.5,
	1, 1.15, 0.5, 1.25, 0.5,
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


double const ring[] = {
	2, 5,
	0.5, 0.8,  0.8, 0.5,  0.5, 0.2,  0.2, 0.5,  0.5, 0.8,
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
	0.00, 0.8,  0.25, 0.2,  0.75, 0.8,  1.00, 0.2,
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
	{"widehat",             angle,        3 },
	{"widetilde",           tilde,        0 },
	{"underbar",            hline,        0 },
	{"underline",           hline,        0 },
	{"overline",            hline,        0 },
	{"underbrace",          brace,        1 },
	{"overbrace",           brace,        3 },
	{"overleftarrow",       arrow,        1 },
	{"overrightarrow",      arrow,        3 },
	{"overleftrightarrow",  udarrow,      1 },
	{"xhookleftarrow",      lhook,        0 },
	{"xhookrightarrow",     rhook,        0 },
	{"xleftarrow",          arrow,        1 },
	{"xLeftarrow",          LArrow,       0 },
	{"xleftharpoondown",    lharpoondown, 0 },
	{"xleftharpoonup",      lharpoonup,   0 },
	{"xleftrightharpoons",  lrharpoons,   0 },
	{"xleftrightarrow",     udarrow,      1 },
	{"xLeftrightarrow",     LRArrow,      0 },
	{"xmapsto",             mapsto,       0 },
	{"xrightarrow",         arrow,        3 },
	{"xRightarrow",         LArrow,       2 },
	{"xrightharpoondown",   lharpoonup,   2 },
	{"xrightharpoonup",     lharpoondown, 2 },
	{"xrightleftharpoons",  rlharpoons,   0 },
	{"underleftarrow",      arrow,        1 },
	{"underrightarrow",     arrow,        3 },
	{"underleftrightarrow", udarrow,      1 },
	{"undertilde",          tilde,        0 },
	{"utilde",              tilde,        0 },

	// Delimiters
	{"(",              parenth,    0 },
	{")",              parenth,    2 },
	{"{",              brace,      0 },
	{"}",              brace,      2 },
	{"lbrace",         brace,      0 },
	{"rbrace",         brace,      2 },
	{"[",              brack,      0 },
	{"]",              brack,      2 },
	{"llbracket",      dbrack,     0 },
	{"rrbracket",      dbrack,     2 },
	{"|",              vert,       0 },
	{"/",              slash,      0 },
	{"slash",          slash,      0 },
	{"vert",           vert,       0 },
	{"lvert",          vert,       0 },
	{"rvert",          vert,       0 },
	{"Vert",           Vert,       0 },
	{"lVert",          Vert,       0 },
	{"rVert",          Vert,       0 },
	{"'",              slash,      1 },
	{"<",              angle,      0 },
	{">",              angle,      2 },
	{"\\",             slash,      1 },
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
	{"ddot",           ddot,       0 },
	{"dddot",          dddot,      0 },
	{"ddddot",         ddddot,     0 },
	{"hat",            angle,      3 },
	{"grave",          slash,      1 },
	{"acute",          slash,      0 },
	{"tilde",          tilde,      0 },
	{"bar",            hline,      0 },
	{"dot",            hlinesmall, 0 },
	{"check",          angle,      1 },
	{"breve",          parenth,    1 },
	{"vec",            arrow,      3 },
	{"mathring",       ring,       0 },

	// Dots
	{"dots",           hline3,     0 },
	{"ldots",          hline3,     0 },
	{"cdots",          hline3,     0 },
	{"vdots",          hline3,     1 },
	{"ddots",          dline3,     0 },
	{"adots",          dline3,     1 },
	{"iddots",         dline3,     1 },
	{"dotsb",          hline3,     0 },
	{"dotsc",          hline3,     0 },
	{"dotsi",          hline3,     0 },
	{"dotsm",          hline3,     0 },
	{"dotso",          hline3,     0 }
};


map<docstring, deco_struct> deco_list;

// sort the table on startup
class init_deco_table {
public:
	init_deco_table() {
		unsigned const n = sizeof(deco_table) / sizeof(deco_table[0]);
		for (named_deco_struct * p = deco_table; p != deco_table + n; ++p) {
			deco_struct d;
			d.data  = p->data;
			d.angle = p->angle;
			deco_list[from_ascii(p->name)] = d;
		}
	}
};

static init_deco_table dummy;


deco_struct const * search_deco(docstring const & name)
{
	map<docstring, deco_struct>::const_iterator p = deco_list.find(name);
	return p == deco_list.end() ? 0 : &(p->second);
}


} // namespace anon


int mathed_font_em(FontInfo const & font)
{
	return theFontMetrics(font).em();
}

/* The math units. Quoting TeX by Topic, p.205:
 *
 * Spacing around mathematical objects is measured in mu units. A mu
 * is 1/18th part of \fontdimen6 of the font in family 2 in the
 * current style, the ‘quad’ value of the symbol font.
 *
 * A \thickmuskip (default value in plain TeX: 5mu plus 5mu) is
 * inserted around (binary) relations, except where these are preceded
 * or followed by other relations or punctuation, and except if they
 * follow an open, or precede a close symbol.
 *
 * A \medmuskip (default value in plain TeX: 4mu plus 2mu minus 4mu)
 * is put around binary operators.
 *
 * A \thinmuskip (default value in plain TeX: 3mu) follows after
 * punctuation, and is put around inner objects, except where these
 * are followed by a close or preceded by an open symbol, and except
 * if the other object is a large operator or a binary relation.
 */

int mathed_thinmuskip(FontInfo font)
{
	font.setFamily(SYMBOL_FAMILY);
	return support::iround(3.0 / 18 * theFontMetrics(font).em());
}


int mathed_medmuskip(FontInfo font)
{
	font.setFamily(SYMBOL_FAMILY);
	return support::iround(4.0 / 18 * theFontMetrics(font).em());
}


int mathed_thickmuskip(FontInfo font)
{
	font.setFamily(SYMBOL_FAMILY);
	return support::iround(5.0 / 18 * theFontMetrics(font).em());
}


int mathed_char_width(FontInfo const & font, char_type c)
{
	return theFontMetrics(font).width(c);
}


int mathed_char_kerning(FontInfo const & font, char_type c)
{
	frontend::FontMetrics const & fm = theFontMetrics(font);
	return fm.rbearing(c) - fm.width(c);
}


void mathed_string_dim(FontInfo const & font,
		       docstring const & s,
		       Dimension & dim)
{
	frontend::FontMetrics const & fm = theFontMetrics(font);
	dim.asc = 0;
	dim.des = 0;
	for (docstring::const_iterator it = s.begin();
	     it != s.end();
	     ++it) {
		dim.asc = max(dim.asc, fm.ascent(*it));
		dim.des = max(dim.des, fm.descent(*it));
	}
	dim.wid = fm.width(s);
}


int mathed_string_width(FontInfo const & font, docstring const & s)
{
	return theFontMetrics(font).width(s);
}


void mathed_draw_deco(PainterInfo & pi, int x, int y, int w, int h,
	docstring const & name)
{
	if (name == ".") {
		pi.pain.line(x + w/2, y, x + w/2, y + h,
			  Color_cursor, Painter::line_onoffdash);
		return;
	}

	deco_struct const * mds = search_deco(name);
	if (!mds) {
		lyxerr << "Deco was not found. Programming error?" << endl;
		lyxerr << "name: '" << to_utf8(name) << "'" << endl;
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
			pi.pain.line(
				int(x + xx + 0.5), int(y + yy + 0.5),
				int(x + x2 + 0.5), int(y + y2 + 0.5),
				pi.base.font.color());
		} else {
			int xp[32];
			int yp[32];
			int const n = int(d[i++]);
			for (int j = 0; j < n; ++j) {
				double xx = d[i++];
				double yy = d[i++];
//	     lyxerr << ' ' << xx << ' ' << yy << ' ';
				if (code == 4)
					sqmt.transform(xx, yy);
				else
					mt.transform(xx, yy);
				xp[j] = int(x + xx + 0.5);
				yp[j] = int(y + yy + 0.5);
				//  lyxerr << "P[" << j ' ' << xx << ' ' << yy << ' ' << x << ' ' << y << ']';
			}
			pi.pain.lines(xp, yp, n, pi.base.font.color());
		}
	}
}


void metricsStrRedBlack(MetricsInfo & mi, Dimension & dim, docstring const & str)
{
	FontInfo font = mi.base.font;
	augmentFont(font, from_ascii("mathnormal"));
	mathed_string_dim(font, str, dim);
}


void drawStrRed(PainterInfo & pi, int x, int y, docstring const & str)
{
	FontInfo f = pi.base.font;
	augmentFont(f, from_ascii("mathnormal"));
	f.setColor(Color_latex);
	pi.pain.text(x, y, str, f);
}


void drawStrBlack(PainterInfo & pi, int x, int y, docstring const & str)
{
	FontInfo f = pi.base.font;
	augmentFont(f, from_ascii("mathnormal"));
	f.setColor(Color_foreground);
	pi.pain.text(x, y, str, f);
}


void math_font_max_dim(FontInfo const & font, int & asc, int & des)
{
	frontend::FontMetrics const & fm = theFontMetrics(font);
	asc = fm.maxAscent();
	des = fm.maxDescent();
}


struct fontinfo {
	string cmd_;
	FontFamily family_;
	FontSeries series_;
	FontShape  shape_;
	ColorCode        color_;
};


FontFamily const inh_family = INHERIT_FAMILY;
FontSeries const inh_series = INHERIT_SERIES;
FontShape  const inh_shape  = INHERIT_SHAPE;


// mathnormal should be the first, otherwise the fallback further down
// does not work
fontinfo fontinfos[] = {
	// math fonts
	{"mathnormal",    ROMAN_FAMILY, MEDIUM_SERIES,
			  ITALIC_SHAPE, Color_math},
	{"mathbf",        inh_family, BOLD_SERIES,
			  inh_shape, Color_math},
	{"mathcal",       CMSY_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"mathfrak",      EUFRAK_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"mathrm",        ROMAN_FAMILY, inh_series,
			  UP_SHAPE, Color_math},
	{"mathsf",        SANS_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"mathbb",        MSB_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"mathtt",        TYPEWRITER_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"mathit",        inh_family, inh_series,
			  ITALIC_SHAPE, Color_math},
	{"mathscr",       RSFS_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"cmex",          CMEX_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"cmm",           CMM_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"cmr",           CMR_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"cmsy",          CMSY_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"eufrak",        EUFRAK_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"msa",           MSA_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"msb",           MSB_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"stmry",         STMARY_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"wasy",          WASY_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"esint",         ESINT_FAMILY, inh_series,
			  inh_shape, Color_math},

	// Text fonts
	{"text",          inh_family, inh_series,
			  inh_shape, Color_foreground},
	{"textbf",        inh_family, BOLD_SERIES,
			  inh_shape, Color_foreground},
	{"textit",        inh_family, inh_series,
			  ITALIC_SHAPE, Color_foreground},
	{"textmd",        inh_family, MEDIUM_SERIES,
			  inh_shape, Color_foreground},
	{"textnormal",    inh_family, inh_series,
			  UP_SHAPE, Color_foreground},
	{"textrm",        ROMAN_FAMILY,
			  inh_series, UP_SHAPE,Color_foreground},
	{"textsc",        inh_family, inh_series,
			  SMALLCAPS_SHAPE, Color_foreground},
	{"textsf",        SANS_FAMILY, inh_series,
			  inh_shape, Color_foreground},
	{"textsl",        inh_family, inh_series,
			  SLANTED_SHAPE, Color_foreground},
	{"texttt",        TYPEWRITER_FAMILY, inh_series,
			  inh_shape, Color_foreground},
	{"textup",        inh_family, inh_series,
			  UP_SHAPE, Color_foreground},

	// TIPA support
	{"textipa",       inh_family, inh_series,
			  inh_shape, Color_foreground},

	// mhchem support
	{"ce",            inh_family, inh_series,
			  inh_shape, Color_foreground},
	{"cf",            inh_family, inh_series,
			  inh_shape, Color_foreground},

	// LyX internal usage
	{"lyxtex",        inh_family, inh_series,
			  UP_SHAPE, Color_latex},
	// FIXME: The following two don't work on OS X, since the Symbol font
	//        uses a different encoding, and is therefore disabled in
	//        FontLoader::available().
	{"lyxsymbol",     SYMBOL_FAMILY, inh_series,
			  inh_shape, Color_math},
	{"lyxboldsymbol", SYMBOL_FAMILY, BOLD_SERIES,
			  inh_shape, Color_math},
	{"lyxblacktext",  ROMAN_FAMILY, MEDIUM_SERIES,
			  UP_SHAPE, Color_foreground},
	{"lyxnochange",   inh_family, inh_series,
			  inh_shape, Color_foreground},
	{"lyxfakebb",     TYPEWRITER_FAMILY, BOLD_SERIES,
			  UP_SHAPE, Color_math},
	{"lyxfakecal",    SANS_FAMILY, MEDIUM_SERIES,
			  ITALIC_SHAPE, Color_math},
	{"lyxfakefrak",   ROMAN_FAMILY, BOLD_SERIES,
			  ITALIC_SHAPE, Color_math}
};


fontinfo * lookupFont(docstring const & name0)
{
	//lyxerr << "searching font '" << name << "'" << endl;
	int const n = sizeof(fontinfos) / sizeof(fontinfo);
	string name = to_utf8(name0);
	for (int i = 0; i < n; ++i)
		if (fontinfos[i].cmd_ == name) {
			//lyxerr << "found '" << i << "'" << endl;
			return fontinfos + i;
		}
	return 0;
}


fontinfo * searchFont(docstring const & name)
{
	fontinfo * f = lookupFont(name);
	return f ? f : fontinfos;
	// this should be mathnormal
	//return searchFont("mathnormal");
}


bool isFontName(docstring const & name)
{
	return lookupFont(name);
}


bool isMathFont(docstring const & name)
{
	fontinfo * f = lookupFont(name);
	return f && f->color_ == Color_math;
}


bool isTextFont(docstring const & name)
{
	fontinfo * f = lookupFont(name);
	return f && f->color_ == Color_foreground;
}


FontInfo getFont(docstring const & name)
{
	FontInfo font;
	augmentFont(font, name);
	return font;
}


void fakeFont(docstring const & orig, docstring const & fake)
{
	fontinfo * forig = searchFont(orig);
	fontinfo * ffake = searchFont(fake);
	if (forig && ffake) {
		forig->family_ = ffake->family_;
		forig->series_ = ffake->series_;
		forig->shape_  = ffake->shape_;
		forig->color_  = ffake->color_;
	} else {
		lyxerr << "Can't fake font '" << to_utf8(orig) << "' with '"
		       << to_utf8(fake) << "'" << endl;
	}
}


void augmentFont(FontInfo & font, docstring const & name)
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		// fake fonts if necessary
		if (!theFontLoader().available(getFont(from_ascii("mathfrak"))))
			fakeFont(from_ascii("mathfrak"), from_ascii("lyxfakefrak"));
		if (!theFontLoader().available(getFont(from_ascii("mathcal"))))
			fakeFont(from_ascii("mathcal"), from_ascii("lyxfakecal"));
	}
	fontinfo * info = searchFont(name);
	if (info->family_ != inh_family)
		font.setFamily(info->family_);
	if (info->series_ != inh_series)
		font.setSeries(info->series_);
	if (info->shape_ != inh_shape)
		font.setShape(info->shape_);
	if (info->color_ != Color_none)
		font.setColor(info->color_);
}


bool isAlphaSymbol(MathAtom const & at)
{
	if (at->asCharInset() ||
	    (at->asSymbolInset() &&
	     at->asSymbolInset()->isOrdAlpha()))
		return true;

	if (at->asFontInset()) {
		MathData const & ar = at->asFontInset()->cell(0);
		for (size_t i = 0; i < ar.size(); ++i) {
			if (!(ar[i]->asCharInset() ||
			      (ar[i]->asSymbolInset() &&
			       ar[i]->asSymbolInset()->isOrdAlpha())))
				return false;
		}
		return true;
	}
	return false;
}


docstring asString(MathData const & ar)
{
	odocstringstream os;
	TexRow texrow(false);
	otexrowstream ots(os,texrow);
	WriteStream ws(ots);
	ws << ar;
	return os.str();
}


void asArray(docstring const & str, MathData & ar, Parse::flags pf)
{
	bool quiet = pf & Parse::QUIET;
	if ((str.size() == 1 && quiet) || (!mathed_parse_cell(ar, str, pf) && quiet))
		mathed_parse_cell(ar, str, pf | Parse::VERBATIM);
}


docstring asString(InsetMath const & inset)
{
	odocstringstream os;
	TexRow texrow(false);
	otexrowstream ots(os,texrow);
	WriteStream ws(ots);
	inset.write(ws);
	return os.str();
}


docstring asString(MathAtom const & at)
{
	odocstringstream os;
	TexRow texrow(false);
	otexrowstream ots(os,texrow);
	WriteStream ws(ots);
	at->write(ws);
	return os.str();
}


} // namespace lyx
