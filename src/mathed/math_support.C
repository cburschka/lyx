#include <config.h>

#include <map>

#include "math_support.h"
#include "lyxfont.h"
#include "math_cursor.h"
#include "math_inset.h"
#include "math_parser.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "frontends/lyx_gui.h"
#include "debug.h"
#include "commandtags.h"
#include "dimension.h"

using std::map;
using std::endl;
using std::max;


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


double const ddot[] = {
	1, 0.2, 0.5,  0.3, 0.5,
	1, 0.7, 0.5,  0.8, 0.5,
	0
};


double const dddot[] = {
	1, 0.1, 0.5,  0.2, 0.5,
	1, 0.45, 0.5, 0.55, 0.5,
	1, 0.8, 0.5,  0.9, 0.5,
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
	{"widehat",             angle,    3 },
	{"widetilde",           tilde,    0 },
	{"underbar",            hline,    0 },
	{"underline",           hline,    0 },
	{"overline",            hline,    0 },
	{"underbrace",          brace,    1 },
	{"overbrace",           brace,    3 },
	{"overleftarrow",       arrow,    1 },
	{"overrightarrow",      arrow,    3 },
	{"overleftrightarrow",  udarrow,  1 },
	{"xleftarrow",          arrow,    1 },
	{"xrightarrow",         arrow,    3 },
	{"underleftarrow",      arrow,    1 },
	{"underrightarrow",     arrow,    3 },
	{"underleftrightarrow", udarrow,  1 },

	// Delimiters
	{"(",              parenth,    0 },
	{")",              parenth,    2 },
	{"{",              brace,      0 },
	{"}",              brace,      2 },
	{"[",              brack,      0 },
	{"]",              brack,      2 },
	{"|",              vert,       0 },
	{"/",              slash,      0 },
	{"vert",           vert,       0 },
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
	{"ddot",           ddot,       0 },
	{"dddot",          dddot,      0 },
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
	{"ldots",          hline3,     0 },
	{"cdots",          hline3,     0 },
	{"vdots",          hline3,     1 },
	{"ddots",          dline3,     0 },
	{"dotsb",          hline3,     0 },
	{"dotsc",          hline3,     0 },
	{"dotsi",          hline3,     0 },
	{"dotsm",          hline3,     0 },
	{"dotso",          hline3,     0 }
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


void mathed_char_dim(LyXFont const & font, unsigned char c, Dimension & dim)
{
	dim.d = font_metrics::descent(c, font);
	dim.a = font_metrics::ascent(c, font);
	dim.w = mathed_char_width(font, c);
}


int mathed_char_ascent(LyXFont const & font, unsigned char c)
{
	return font_metrics::ascent(c, font);
}


int mathed_char_descent(LyXFont const & font, unsigned char c)
{
	return font_metrics::descent(c, font);
}


int mathed_char_width(LyXFont const & font, unsigned char c)
{
	return font_metrics::width(c, font);
}


void mathed_string_dim(LyXFont const & font, string const & s, Dimension & dim)
{
#if 1
	dim.a = 0;
	dim.d = 0;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		dim.a = max(dim.a, font_metrics::ascent(*it, font));
		dim.d = max(dim.d, font_metrics::descent(*it, font));
	}
#else
	dim.a = font_metrics::maxAscent(font);
	dim.d = font_metrics::maxDescent(font);
#endif
	dim.w = font_metrics::width(s, font);
}


int mathed_string_width(LyXFont const & font, string const & s)
{
	return font_metrics::width(s, font);
}


void mathed_draw_deco(MathPainterInfo & pi, int x, int y, int w, int h,
	const string & name)
{
	if (name == ".") {
		pi.pain.line(x + w/2, y, x + w/2, y + h,
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
			pi.pain.line(
				x + int(xx + 0.5), y + int(yy + 0.5),
				x + int(x2 + 0.5), y + int(y2 + 0.5),
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
				xp[j] = x + int(xx + 0.5);
				yp[j] = y + int(yy + 0.5);
				//  lyxerr << "P[" << j " " << xx << " " << yy << " " << x << " " << y << "]";
			}
			pi.pain.lines(xp, yp, n, LColor::math);
		}
	}
}


void mathed_draw_framebox(MathPainterInfo & pi, int x, int y, MathInset const * p)
{
	if (mathcursor && mathcursor->isInside(p))
		pi.pain.rectangle(x, y - p->ascent(), p->width(), p->height(),
			LColor::mathframe);
}


// In the future maybe we use a better fonts renderer
void drawStr(MathPainterInfo & pi, LyXFont const & font,
	int x, int y, string const & str)
{
	pi.pain.text(x, y, str, font);
}


void drawStrRed(MathPainterInfo & pi, int x, int y, string const & str)
{
	LyXFont f = pi.base.font;
	f.setColor(LColor::latex);
	pi.pain.text(x, y, str, f);
}


void drawStrBlack(MathPainterInfo & pi, int x, int y, string const & str)
{
	LyXFont f = pi.base.font;
	f.setColor(LColor::black);
	pi.pain.text(x, y, str, f);
}


void drawChar(MathPainterInfo & pi, LyXFont const & font, int x, int y, char c)
{
	pi.pain.text(x, y, c, font);
}


void math_font_max_dim(LyXFont const & font, int & asc, int & des)
{
	asc = font_metrics::maxAscent(font);
	des = font_metrics::maxDescent(font);
}


struct fontinfo {
	string cmd_;
	LyXFont::FONT_FAMILY family_;
	LyXFont::FONT_SERIES series_;
	LyXFont::FONT_SHAPE  shape_;
	LColor::color        color_;
};


LyXFont::FONT_FAMILY const inh_family = LyXFont::INHERIT_FAMILY;
LyXFont::FONT_SERIES const inh_series = LyXFont::INHERIT_SERIES;
LyXFont::FONT_SHAPE  const inh_shape  = LyXFont::INHERIT_SHAPE; 


// mathnormal should be the first, otherwise the fallback fuerther down
// does not work
fontinfo fontinfos[] = {
	{"mathnormal", inh_family, LyXFont::MEDIUM_SERIES, LyXFont::UP_SHAPE, LColor::math},
	{"mathbf", inh_family, LyXFont::BOLD_SERIES, inh_shape, LColor::math},
	{"mathcal",LyXFont::CMSY_FAMILY, inh_series, inh_shape, LColor::math},
	{"mathfrak", LyXFont::EUFRAK_FAMILY, inh_series, inh_shape, LColor::math},
	{"mathrm", LyXFont::ROMAN_FAMILY, inh_series, inh_shape, LColor::math},
	{"mathsf", LyXFont::SANS_FAMILY, inh_series, inh_shape, LColor::math},
	{"cmex",   LyXFont::CMEX_FAMILY, inh_series, inh_shape, LColor::math},
	{"cmm",    LyXFont::CMM_FAMILY, inh_series, inh_shape, LColor::math},
	{"cmr",    LyXFont::CMR_FAMILY, inh_series, inh_shape, LColor::math},
	{"cmsy",   LyXFont::CMSY_FAMILY, inh_series, inh_shape, LColor::math},
	{"eufrak", LyXFont::EUFRAK_FAMILY, inh_series, inh_shape, LColor::math},
	{"msa",    LyXFont::MSA_FAMILY, inh_series, inh_shape, LColor::math},
	{"msb",    LyXFont::MSB_FAMILY, inh_series, inh_shape, LColor::math},
	{"wasy",   LyXFont::WASY_FAMILY, inh_series, inh_shape, LColor::math},
	{"text",   inh_family, inh_series, inh_shape, LColor::black},
	{"textbf", inh_family, LyXFont::BOLD_SERIES, inh_shape, LColor::black},
	{"textit", inh_family, inh_series, LyXFont::ITALIC_SHAPE, LColor::black},
	{"textmd", inh_family, LyXFont::MEDIUM_SERIES, inh_shape, LColor::black},
	{"textnormal", inh_family, inh_series, LyXFont::UP_SHAPE, LColor::black},
	{"textrm", LyXFont::ROMAN_FAMILY, inh_series,LyXFont::UP_SHAPE,LColor::black},
	{"textsc", inh_family, inh_series, LyXFont::SMALLCAPS_SHAPE, LColor::black},
	{"textsf", LyXFont::SANS_FAMILY, inh_series, inh_shape, LColor::black},
	{"textsl", inh_family, inh_series, LyXFont::SLANTED_SHAPE, LColor::black},
	{"texttt", LyXFont::TYPEWRITER_FAMILY, inh_series, inh_shape, LColor::black},
	{"textup", inh_family, inh_series, LyXFont::UP_SHAPE, LColor::black},

	// TIPA support
	{"textipa",   inh_family, inh_series, inh_shape, LColor::black},

	{"lyxtex", inh_family, inh_series, inh_shape, LColor::latex},
	{"lyxsymbol", LyXFont::SYMBOL_FAMILY, inh_series, inh_shape, LColor::math},
	{"lyxboldsymbol",
		LyXFont::SYMBOL_FAMILY, LyXFont::BOLD_SERIES, inh_shape, LColor::math},
	{"lyxitsymbol", LyXFont::SYMBOL_FAMILY,
		inh_series, LyXFont::ITALIC_SHAPE, LColor::math},
	{"lyxblacktext", LyXFont::ROMAN_FAMILY,
		LyXFont::MEDIUM_SERIES, LyXFont::UP_SHAPE, LColor::black},
	{"lyxnochange", inh_family, inh_series, inh_shape, LColor::black},

	{"lyxfakebb", LyXFont::TYPEWRITER_FAMILY, LyXFont::BOLD_SERIES,
		LyXFont::UP_SHAPE, LColor::math},
	{"lyxfakecal", LyXFont::SANS_FAMILY, LyXFont::MEDIUM_SERIES,
		LyXFont::ITALIC_SHAPE, LColor::math},
	{"lyxfakefrak", LyXFont::ROMAN_FAMILY, LyXFont::BOLD_SERIES,
		LyXFont::ITALIC_SHAPE, LColor::math}
};


fontinfo * lookupFont(string const & name)
{
	//lyxerr << "searching font '" << name << "'\n"; 
	int const n = sizeof(fontinfos) / sizeof(fontinfo);
	for (int i = 0; i < n; ++i)
		if (fontinfos[i].cmd_ == name) {
			//lyxerr << "found '" << i << "'\n"; 
			return fontinfos + i;
		}
	return 0;
}


fontinfo * searchFont(string const & name)
{
	fontinfo * f = lookupFont(name);
	return f ? f : fontinfos;
	// this should be mathnormal
	//return searchFont("mathnormal");
}


bool isFontName(string const & name)
{
	return lookupFont(name);
}


LyXFont getFont(string const & name)
{
	LyXFont font;
	augmentFont(font, name);
	return font;
}


void fakeFont(string const & orig, string const & fake)
{
	fontinfo * forig = searchFont(orig);
	fontinfo * ffake = searchFont(fake);
	if (forig && ffake) {
		forig->family_ = ffake->family_;
		forig->series_ = ffake->series_;
		forig->shape_  = ffake->shape_;
		forig->color_  = ffake->color_;
	} else {
		lyxerr << "Can't fake font '" << orig << "' with '" << fake << "'\n";
	}
}


void augmentFont(LyXFont & font, string const & name)
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;

		// fake fonts if necessary
		if (!lyx_gui::font_available(getFont("mathfrak")))
			fakeFont("mathfrak", "lyxfakefrak");
		if (!lyx_gui::font_available(getFont("mathcal")))
			fakeFont("mathcal", "lyxfakecal");
	}
	fontinfo * info = searchFont(name);
	if (info->family_ != inh_family)
		font.setFamily(info->family_);
	if (info->series_ != inh_series)
		font.setSeries(info->series_);
	if (info->shape_ != inh_shape)
		font.setShape(info->shape_);
	if (info->color_ != LColor::none)
		font.setColor(info->color_);
}
