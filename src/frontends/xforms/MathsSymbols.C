/**
 * \file MathsSymbols.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 */

#include <config.h>
#include <algorithm>
 
#include XPM_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/LAssert.h" 
#include "support/lstrings.h"
#include "debug.h"
#include "MathsSymbols.h"
#include "FormMathsPanel.h"
 
using std::max;
using std::endl;
using std::ostream;

#ifndef CXX_GLOBAL_CSTD
using std::strstr;
#endif


/* Latex code for those bitmaps */

#include "greek.xbm"
#include "arrows.xbm"
#include "brel.xbm"
#include "bop.xbm"
#include "misc.xbm"
#include "varsz.xbm"
#include "dots.xbm"
#include "mathed/math_parser.h"
#include "frac.xpm"
#include "sub.xpm"
#include "super.xpm"
#include "style.xpm"
#include "sqrt.xpm"
#include "delim.xbm"
#include "delim.xpm"
#include "deco.xbm"
#include "deco.xpm"
#include "space.xpm"
#include "matrix.xpm"
#include "equation.xpm"

char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh"
};

int const nr_function_names = sizeof(function_names) /
	       			     sizeof(char const *);

char const * latex_arrow[] = {
	"downarrow", "leftarrow", "Downarrow", "Leftarrow",
	"hookleftarrow", "rightarrow", "uparrow", "Rightarrow", "Uparrow",
	"hookrightarrow", "updownarrow", "Leftrightarrow", "leftharpoonup",
	"rightharpoonup", "rightleftharpoons", "leftrightarrow", "Updownarrow",
	"leftharpoondown", "rightharpoondown", "mapsto",
	"Longleftarrow", "Longrightarrow", "Longleftrightarrow",
	"longleftrightarrow", "longleftarrow", "longrightarrow", "longmapsto",
	"nwarrow", "nearrow", "swarrow", "searrow",  "",
};

int const nr_latex_arrow = sizeof(latex_arrow) / sizeof(char const *);

char const * latex_bop[] = {
	"pm", "cap", "diamond", "oplus",
	"mp", "cup", "bigtriangleup", "ominus",
	"times", "uplus", "bigtriangledown", "otimes",
	"div", "sqcap", "triangleright", "oslash",
	"cdot", "sqcup", "triangleleft", "odot",
	"star", "vee", "amalg", "bigcirc",
	"setminus", "wedge", "dagger", "circ",
	"bullet", "wr", "ddagger", ""
};

int const nr_latex_bop = sizeof(latex_bop) / sizeof(char const *);

char const * latex_brel[] = {
	"leq", "geq", "equiv", "models",
	"prec", "succ", "sim", "perp",
	"preceq", "succeq", "simeq", "mid",
	"ll", "gg", "asymp", "parallel",
	"subset", "supset", "approx", "smile",
	"subseteq", "supseteq", "cong", "frown",
	"sqsubseteq", "sqsupseteq", "doteq", "neq",
	"in", "ni", "propto", "notin",
	"vdash", "dashv", "bowtie", ""
};

int const nr_latex_brel = sizeof(latex_brel) / sizeof(char const *);

char const * latex_dots[] = {
	"ldots", "cdots", "vdots", "ddots"
};

int const nr_latex_dots = sizeof(latex_dots) / sizeof(char const *);

char const * latex_greek[] = {
	"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi",
	"Sigma", "Upsilon", "Phi", "Psi", "Omega",
	"alpha", "beta", "gamma", "delta", "epsilon", "varepsilon", "zeta",
	"eta", "theta", "vartheta", "iota", "kappa", "lambda", "mu",
	"nu", "xi", "pi", "varpi", "rho", "sigma", "varsigma",
	"tau", "upsilon", "phi", "varphi", "chi", "psi", "omega", ""
};

int const nr_latex_greek = sizeof(latex_greek) / sizeof(char const *);

char const * latex_misc[] = {
	"nabla", "partial", "infty", "prime", "ell",
	"emptyset", "exists", "forall", "imath",  "jmath",
	"Re", "Im", "aleph", "wp", "hbar",
	"angle", "top", "bot", "Vert", "neg",
	"flat", "natural", "sharp", "surd", "triangle",
	"diamondsuit", "heartsuit", "clubsuit", "spadesuit", 
	"mathbb N", "mathbb Z", "mathbb Q", 
	"mathbb R", "mathbb C", "mathbb H",
	"mathcal F", "mathcal L", 
	"mathcal H", "mathcal O",""
};

int const nr_latex_misc = sizeof(latex_misc) / sizeof(char const *);

char const * latex_varsz[] = {
	"sum", "int", "oint",
	"prod", "coprod", "bigsqcup",
	"bigotimes", "bigodot", "bigoplus",
	"bigcap", "bigcup", "biguplus",
	"bigvee", "bigwedge", ""
};

int const nr_latex_varsz = sizeof(latex_varsz) / sizeof(char const *);

static char const ** mathed_get_pixmap_from_icon(int d)
{
	switch (d) {
		case MM_FRAC: return frac;
		case MM_SQRT: return sqrt_xpm;
		case MM_SUPER: return super_xpm;
		case MM_SUB: return sub_xpm;
		case MM_STYLE: return style_xpm;
		case MM_DELIM: return delim;
		case MM_MATRIX: return matrix;
		case MM_EQU: return equation;
		case MM_DECO: return deco;
		case MM_SPACE: return space_xpm;
		default: return 0;
	}
}
 
static char const ** pixmapFromBitmapData(char const * s, int wx, int hx)
{
	char const ** data = 0;

	int id = -1;

	int i = 0;
	for (; i < 6; ++i) {
		char const ** latex_str = 0;
		switch (i) {
		case 0: latex_str = latex_greek; break;
		case 1: latex_str = latex_bop; break;
		case 2: latex_str = latex_brel; break;
		case 3: latex_str = latex_arrow; break;
		case 4: latex_str = latex_varsz; break;
		case 5: latex_str = latex_misc; break;
		}

		for (int k = 0; latex_str[k][0] > ' '; ++k) {
			if (compare(latex_str[k], s) == 0) {
				id = k;
				break;
			}
		}
		if (id >= 0) break;
	}
	if (i < 6 && id >= 0) {
		unsigned char const * bdata = 0;
		int w = 0;
		int h = 0;
		int dw = 0;
		int dh = 0;

		lyxerr[Debug::MATHED] << "Imando " << i << ", " << id << endl;
		switch (i) {
		case 0:
			if (id <= 10) {
				w = Greek_width;
				h = Greek_height;
				bdata = Greek_bits;
				dw = 6;
				dh = 2;
			} else {
				w = greek_width;
				h = greek_height;
				bdata = greek_bits;
				dw = 7;
				dh = 4;
				id -= 11;
			}
			break;
		case 1:
			w = bop_width;
			h = bop_height;
			bdata = bop_bits;
			dw = 4;
			dh = 8;
			break;
		case 2:
			w = brel_width;
			h = brel_height;
			bdata = brel_bits;
			dw = 4;
			dh = 9;
			break;
		case 3:
			if (id < 20) {
				w = arrow_width;
				h = arrow_height;
				bdata = arrow_bits;
				dw = 5;
				dh = 4;
			} else if (id > 28) {
				w = darrow_width;
				h = darrow_height;
				bdata = darrow_bits;
				dw = 2;
				dh = 2;
				id -= 29;
			} else {
				w = larrow_width;
				h = larrow_height;
				bdata = larrow_bits;
				dw = 2;
				dh = 4;
				id -= 20;
			}
			break;
		case 4:
			w = varsz_width;
			h = varsz_height;
			bdata = varsz_bits;
			dw = 3;
			dh = 5;
			break;
		case 5:
			if (id < 29) {
				w = misc_width;
				h = misc_height;
				bdata = misc_bits;
				dw = 5;
				dh = 6;
			} else if (id > 36) {
				w = misc3_width;
				h = misc3_height;
				bdata = misc3_bits;
				dw = 3;
				dh = 2;
				id -= 37;
			} else {
				w = misc2_width;
				h = misc2_height;
				bdata = misc2_bits;
				dw = 2;
				dh = 2;
				id -= 29;
			}
			break;
		}
		int ww = w / dw;
		int hh = h / dh;
		XImage * xima = XCreateImage(fl_get_display(), 0, 1, XYBitmap, 0,
					     const_cast<char*>(reinterpret_cast<char const *>(bdata)), w, h, 8, 0);
		xima->byte_order = LSBFirst;
		xima->bitmap_bit_order = LSBFirst;
		int x = (id % dw) * ww;
		int y = (id/dw) * hh;
		if (ww > wx) ww = wx;
		if (hh > hx) hh = hx;
		XImage * sbima = XSubImage(xima, x, y, ww, hh);
		XpmCreateDataFromImage(fl_get_display(), const_cast<char***>(&data), sbima, sbima, 0);

		// Dirty hack to get blue symbols quickly
		char * sx = const_cast<char*>(strstr(data[2], "FFFFFFFF"));
		if (sx) {
			for (int k = 0; k < 8; ++k) sx[k] = '0';
		}

//	XDestroyImage(xima);
	}

	return data;
}

 
char const ** get_pixmap_from_symbol(char const * arg, int wx, int hx)
{
	lyx::Assert(arg);
	
	char const ** data = 0;
	latexkeys const * l = in_word_set(arg);
	if (!l)
		return 0;

	switch (l->token) {
	case LM_TK_FRAC:
		data = mathed_get_pixmap_from_icon(MM_FRAC);
		break;
	case LM_TK_SQRT:
		data = mathed_get_pixmap_from_icon(MM_SQRT);
		break;
	case LM_TK_SYM:
	case LM_TK_CMR:
	case LM_TK_CMSY:
	case LM_TK_CMEX:
	case LM_TK_CMM:
	case LM_TK_MSA:
	case LM_TK_MSB:
		// I have to use directly the bitmap data since the
		// bitmap tables are not yet created when this
		// function is called.
		data = pixmapFromBitmapData(arg, wx, hx);
		break;
	default:
		break;
	}

	return data;
}
