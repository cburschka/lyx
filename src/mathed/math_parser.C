/*
 *  File:        math_parser.C
 *  Purpose:     Parser for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Parse LaTeX2e math mode code.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include <cctype>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_parser.h"
#include "array.h"
#include "math_inset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_root.h"
#include "math_arrayinset.h"
#include "math_sqrtinset.h"
#include "math_matrixinset.h"
#include "math_accentinset.h"
#include "math_bigopinset.h"
#include "math_funcinset.h"
#include "math_spaceinset.h"
#include "math_sizeinset.h"
#include "math_dotsinset.h"
#include "math_fracinset.h"
#include "math_deliminset.h"
#include "math_decorationinset.h"
#include "debug.h"
#include "mathed/support.h"
#include "lyxlex.h"

using std::istream;
using std::endl;



// These are lexical codes, not semantic
enum lexcode_enum {
	LexNone,
	LexESC,
	LexAlpha,
	LexDigit,
	LexBOP,         // Binary operators or relations
	LexMathSpace,
	LexOpen,
	LexClose,
	LexComment,
	LexArgument,
	LexSpace,
	LexNewLine,
	LexOther,
	LexSelf
};

lexcode_enum lexcode[256];  



char const * latex_special_chars = "#$%&_{}";


/// Read TeX into data, flags give stop conditions
void mathed_parse(MathArray & data, unsigned flags);


namespace {

const char LM_TK_OPEN  = '{';
const char LM_TK_CLOSE = '}';

enum {
	FLAG_BRACE      = 1 << 0,  //  A { needed              //}
	FLAG_BRACE_OPT  = 1 << 2,  //  Optional {              
	FLAG_BRACE_LAST = 1 << 3,  //  Last } ends the parsing process
	FLAG_BRACK_ARG  = 1 << 4,  //  Optional [     
	FLAG_RIGHT      = 1 << 5,  //  Next right ends the parsing process
	FLAG_END        = 1 << 6,  //  Next end ends the parsing process
	FLAG_BRACE_FONT = 1 << 7,  //  Next } closes a font
	FLAG_BRACK_END  = 1 << 9,  //  Next ] ends the parsing process
	FLAG_AMPERSAND  = 1 << 10, //  Next & ends the parsing process
	FLAG_NEWLINE    = 1 << 11  //  Next \\ ends the parsing process
};


///
union {
	///
	int i;
	///
	latexkeys const * l;
} yylval;


string yytext;

int yylineno;
istream * yyis;
bool yy_mtextmode = false;



struct latex_mathenv_type {
	char const *      name;
	char const *      basename;
	MathInsetTypes    typ;
	bool              numbered;
	bool              ams;
};

latex_mathenv_type latex_mathenv[] = { 
	{"math",         "math",         LM_OT_SIMPLE,   0, 0},
	{"equation*",    "equation",     LM_OT_EQUATION, 0, 0},
	{"equation",     "equation",     LM_OT_EQUATION, 1, 0},
	{"eqnarray*",    "eqnarray",     LM_OT_EQNARRAY, 0, 0},
	{"eqnarray",     "eqnarray",     LM_OT_EQNARRAY, 1, 0},
	{"align*",       "align",        LM_OT_ALIGN,    0, 1},
	{"align",        "align",        LM_OT_ALIGN,    1, 1},
	{"alignat*",     "alignat",      LM_OT_ALIGNAT,  0, 1},
	{"alignat",      "alignat",      LM_OT_ALIGNAT,  1, 1},
	{"multline*",    "multline",     LM_OT_MULTLINE, 0, 1},
	{"multline",     "multline",     LM_OT_MULTLINE, 1, 1},
	{"array",        "array",        LM_OT_MATRIX,   0, 1}
};

int const latex_mathenv_num = sizeof(latex_mathenv)/sizeof(latex_mathenv[0]);



void mathPrintError(string const & msg) 
{
	lyxerr << "Line ~" << yylineno << ": Math parse error: " << msg << endl;
}


void LexInitCodes()
{
	for (int i = 0; i <= 255; ++i) {
		if (isalpha(i))
			lexcode[i] = LexAlpha;
		else if (isdigit(i))
			lexcode[i] = LexDigit;
		else if (isspace(i))
			lexcode[i] = LexSpace;
		else
			lexcode[i] = LexNone;
	}
	
	lexcode['\t'] = lexcode['\f'] = lexcode[' '] = LexSpace;
	lexcode['\n'] = LexNewLine;
	lexcode['%'] = LexComment;
	lexcode['#'] = LexArgument;
	lexcode['+'] = lexcode['-'] = lexcode['*'] = lexcode['/']
		= lexcode['<'] = lexcode['>'] = lexcode['='] = LexBOP;
	
	lexcode['!'] = lexcode[','] = lexcode[':']
		= lexcode[';'] = LexMathSpace;
	
	lexcode['('] = lexcode[')'] = lexcode['|'] = lexcode['.'] =
		lexcode['?'] = LexOther; 
	
	lexcode['\''] = lexcode['@'] = LexAlpha;
	
	lexcode['['] = lexcode[']'] = lexcode['^'] = lexcode['_'] = 
		lexcode['&'] = LexSelf;  
	
	lexcode['\\'] = LexESC;
	lexcode['{'] = LexOpen;
	lexcode['}'] = LexClose;
}


char LexGetArg(char lf, bool accept_spaces = false)
{
	while (yyis->good()) {
		char c;
		yyis->get(c);
		if (c > ' ') {
			if (!lf) 
				lf = c;
			else if (c != lf) {
				lyxerr << "Math parse error: unexpected '" << c << "'" << endl;
				return '\0';
			}
			break;
		}
	}
	char rg = 0;
	if (lf == '{') rg = '}';
	if (lf == '[') rg = ']';
	if (lf == '(') rg = ')';
	if (!rg) {
		lyxerr << "Math parse error: unknown bracket '" << lf << "'" << endl;
		return '\0';
	}
	yytext.erase();
	int bcnt = 1;
	do {
		char c;
		yyis->get(c);
		if (c == lf) ++bcnt;
		if (c == rg) --bcnt;
		if ((c > ' ' || (c == ' ' && accept_spaces)) && bcnt > 0)
			yytext += c;
	} while (bcnt > 0 && yyis->good());

	return rg;
}


int yylex()
{
	static int init_done;
	
	if (!init_done) LexInitCodes();
	
	while (yyis->good()) {
		char c;
		yyis->get(c);
		
		if (yy_mtextmode && c == ' ') {
			yylval.i= ' ';
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexNewLine) {
			++yylineno; 
			continue;
		} else if (lexcode[c] == LexComment) {
			do {
				yyis->get(c);
			} while (c != '\n' && yyis->good());  // eat comments
		} else if (lexcode[c] == LexDigit
			   || lexcode[c] == LexOther
			   || lexcode[c] == LexMathSpace) {
			yylval.i = c;
			return LM_TK_STR;
		} else if (lexcode[c] == LexAlpha) {
			yylval.i = c;
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexBOP) {
			yylval.i = c;
			return LM_TK_BOP;
		} else if (lexcode[c] == LexSelf) {
			return c;
		} else if (lexcode[c] == LexArgument) {
			yyis->get(c);
			yylval.i = c - '0';
			return LM_TK_ARGUMENT; 
		} else if (lexcode[c] == LexOpen) {
			return LM_TK_OPEN;
		} else if (lexcode[c] == LexClose) {
			return LM_TK_CLOSE;
		} else if (lexcode[c] == LexESC)   {
			yyis->get(c);
			if (c == '\\')	{
				yylval.i = -1;
				return LM_TK_NEWLINE;
			}
			if (c == '(') {
				yylval.i = LM_OT_SIMPLE;
				return LM_TK_BEGIN;
			}
			if (c == ')') {
				yylval.i = LM_OT_SIMPLE;
				return LM_TK_END;
			}
			if (c == '[') {
				yylval.i = LM_OT_EQUATION;
				return LM_TK_BEGIN;
			}
			if (c == ']') {
				yylval.i = LM_OT_EQUATION;
				return LM_TK_END;
			}
			if (contains(latex_special_chars, c)) {
				yylval.i = c;
				return LM_TK_SPECIAL;
			} 
			if (lexcode[c] == LexMathSpace) {
				int i;
				for (i = 0; i < 4 && static_cast<int>(c) != latex_mathspace[i][0]; ++i)
					;
				yylval.i = (i < 4) ? i : 0; 
				return LM_TK_SPACE; 
			}
			if (lexcode[c] == LexAlpha || lexcode[c] == LexDigit) {
				yytext.erase();
				while (lexcode[c] == LexAlpha || lexcode[c] == LexDigit) {
					yytext += c;
					yyis->get(c);
				}
				if (yyis->good())
					yyis->putback(c);
				lyxerr << "reading: text '" << yytext << "'\n";
				latexkeys const * l = in_word_set(yytext);
				if (!l)
					return LM_TK_UNDEF;

				if (l->token == LM_TK_BEGIN || l->token == LM_TK_END) { 
					LexGetArg('{');
					int i = 0;
					while (i < latex_mathenv_num && yytext != latex_mathenv[i].name)
						 ++i;
					yylval.i = i;
				} else if (l->token == LM_TK_SPACE) 
					yylval.i = l->id;
				else
					yylval.l = l;
				return l->token;
			}
		}
	}
	return 0;
}


// Accent hacks only for 0.12. Stolen from Cursor.
int accent = 0;
int nestaccent[8];

void setAccent(int ac)
{
	if (ac > 0 && accent < 8)
		nestaccent[accent++] = ac;
	else
	  accent = 0;  // consumed!
}


MathInset * doAccent(byte c, MathTextCodes t)
{
	MathInset * ac = 0;
	
	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
		  ac = new MathAccentInset(c, t, nestaccent[i]);
		else 
		  ac = new MathAccentInset(ac, nestaccent[i]);
	}
	accent = 0;  // consumed!
	
	return ac;
}


MathInset * doAccent(MathInset * p)
{
	MathInset * ac = 0;
	
	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
		  ac = new MathAccentInset(p, nestaccent[i]);
		else 
		  ac = new MathAccentInset(ac, nestaccent[i]);
	}
	accent = 0;  // consumed!
	
	return ac;
}


void do_insert(MathArray & dat, MathInset * m)
{
	if (accent) 
		dat.push_back(doAccent(m));
	else
		dat.push_back(m);
}

void do_insert(MathArray & dat, byte ch, MathTextCodes fcode)
{
	if (accent) 
		dat.push_back(doAccent(ch, fcode));
	else
		dat.push_back(ch, fcode);
}


void handle_frac(MathArray & dat, MathInsetTypes t)
{
	MathFracInset * p = new MathFracInset(t);
	mathed_parse(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
	mathed_parse(p->cell(1), FLAG_BRACE | FLAG_BRACE_LAST);
	dat.push_back(p);
}


MathScriptInset * lastScriptInset(MathArray & array)
{
	MathInset * p = array.back_inset();
	if (!p || !p->isScriptInset()) {
		p = new MathScriptInset;
		array.push_back(p);
	}
	return static_cast<MathScriptInset *>(p);
}

}



static bool   curr_num;
static string curr_label;

void mathed_parse_lines(MathInset * inset, int col, bool numbered, bool outmost)
{
	MathGridInset * p = static_cast<MathGridInset *>(inset);
	for (int row = 0; true; ++row) {
		// reset global variables
		curr_num   = numbered;
		curr_label = string();

		// reading a row
		int idx = p->nargs() - p->ncols();
		for (int i = 0; i < col - 1; ++i, ++idx)
			mathed_parse(p->cell(idx), FLAG_AMPERSAND);
		mathed_parse(p->cell(idx), FLAG_NEWLINE | FLAG_END);

		if (outmost) {
			MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
			m->numbered(row, curr_num);
			m->label(row, curr_label);
		}

		// Hack!
		// no newline
		if (yylval.i != -1)
			break;

		p->appendRow();
	}
}


MathInset * mathed_parse()
{
	MathInset * p = 0;
	int t = yylex();

	switch (t) {
		case LM_TK_NEWCOMMAND: {
			LexGetArg('{');
			string name = yytext.substr(1);
			
			int na = 0; 
			char const c = yyis->peek();
			if (c == '[') {
				LexGetArg('[');
				na = atoi(yytext.c_str());
			} 

			p = new MathMacroTemplate(name, na);
			mathed_parse(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
			lyxerr << "LM_TK_NEWCOMMAND: name: " << name << " na: " << na << "\n";
			break;
		}

		case LM_TK_BEGIN: {
			int i = yylval.i;
			lyxerr << "reading math environment " << i << " "
				<< latex_mathenv[i].name << "\n";

			MathInsetTypes typ = latex_mathenv[i].typ;
			p = new MathMatrixInset(typ);
			switch (typ) {

				case LM_OT_SIMPLE: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label = string();
					mathed_parse(p->cell(0), 0);
					MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
					m->numbered(0, curr_num);
					m->label(0, curr_label);
					break;
				}

				case LM_OT_EQUATION: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label = string();
					mathed_parse(p->cell(0), FLAG_END);
					MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
					m->numbered(0, curr_num);
					m->label(0, curr_label);
					break;
				}

				case LM_OT_EQNARRAY: {
					mathed_parse_lines(p, 3, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGNAT: {
					LexGetArg('{');
					//int c = atoi(yytext.c_str());
					lyxerr << "LM_OT_ALIGNAT: not implemented\n";
					mathed_parse_lines(p, 2, latex_mathenv[i].numbered, true);
					lyxerr << "LM_OT_ALIGNAT: par: " << *p << "\n";
					break;
				}

				default: 
					lyxerr << "1: unknown math environment: " << typ << "\n";
			}

			p->SetName(latex_mathenv[i].basename);

			break;
		}
		
		default:
			lyxerr << "2 unknown math environment: " << t << "\n";
	}

	return p;
}


void mathed_parse(MathArray & array, unsigned flags)
{
	int  t = yylex();
	int  tprev = 0;
	bool panic = false;
	static int plevel = -1;
	MathTextCodes varcode = LM_TC_VAR;
	
	int brace = 0;
	int acc_brace = 0;
	int acc_braces[8];

	++plevel;
	while (t) {
		//lyxerr << "t: " << t << " flags: " << flags;
		//array.dump(lyxerr);
		//lyxerr << "\n";

		if ((flags & FLAG_BRACE) && t != LM_TK_OPEN) {
			if (!(flags & FLAG_BRACK_ARG) || t != '[') {
				mathPrintError(
					"Expected {. Maybe you forgot to enclose an argument in {}");
				panic = true;
				break;
			}
		}

		switch (t) {
			
		case LM_TK_ALPHA:
			do_insert(array, yylval.i, varcode);
			break;

		case LM_TK_ARGUMENT:
			array.push_back(new MathMacroArgument(yylval.i));
			break;

		case LM_TK_SPECIAL:
			array.push_back(yylval.i, LM_TC_SPECIAL);
			break;

		case LM_TK_STR:
			do_insert(array, yylval.i, LM_TC_CONST);
			break;

		case LM_TK_OPEN:
			++brace;
			if (accent && tprev == LM_TK_ACCENT) {
				acc_braces[acc_brace++] = brace;
				break;
			}
			if (flags & FLAG_BRACE_OPT) {
				flags &= ~FLAG_BRACE_OPT;
				flags |= FLAG_BRACE;
			}
			
			if (flags & FLAG_BRACE)
				flags &= ~FLAG_BRACE;
			else 
				array.push_back('{', LM_TC_TEX);
			break;

		case LM_TK_CLOSE:
			--brace;	 
			if (brace < 0) {
				mathPrintError("Unmatching braces");
				panic = true;
				break;
			}
			if (acc_brace && brace == acc_braces[acc_brace - 1] - 1) {
				--acc_brace;
				break;
			}
			if (flags & FLAG_BRACE_FONT) {
				varcode = LM_TC_VAR;
				yy_mtextmode = false;
				flags &= ~FLAG_BRACE_FONT;
				break;
			}
			if (brace == 0 && (flags & FLAG_BRACE_LAST)) {
				--plevel;
				return;
			}
			array.push_back('}', LM_TC_TEX);
			break;
		
		case '[':
			if (flags & FLAG_BRACK_ARG) {
				flags &= ~FLAG_BRACK_ARG;
				char const rg = LexGetArg('[');
				if (rg != ']') {
					mathPrintError("Expected ']'");
					panic = true;
					break;
				}	   
			} else
				array.push_back('[', LM_TC_CONST);
			break;

		case ']':
			if (flags & FLAG_BRACK_END) {
				--plevel;
				return;
			}
			array.push_back(']', LM_TC_CONST);
			break;
		
		case '^':
		{
			MathArray ar;
			mathed_parse(ar, FLAG_BRACE_OPT | FLAG_BRACE_LAST);
			MathScriptInset * p = lastScriptInset(array);
			p->setData(ar, 0);
			p->up(true);
			break;
		}
		
		case '_':
		{
			MathArray ar;
			mathed_parse(ar, FLAG_BRACE_OPT | FLAG_BRACE_LAST);
			MathScriptInset * p = lastScriptInset(array);
			p->setData(ar, 1);
			p->down(true);
			break;
		}
		
		case LM_TK_LIMIT:
		{
			MathScriptInset * p = lastScriptInset(array);
			if (p) 
				p->limits(yylval.l->id ? 1 : -1);
			break;
		}
		
		case '&':
		{
			if (flags & FLAG_AMPERSAND) {
				flags &= ~FLAG_AMPERSAND;
				--plevel;
				return;
			}
			lyxerr << "found tab unexpectedly, array: '" << array << "'\n";
			break;
		}
		
		case LM_TK_NEWLINE:
		{
			if (flags & FLAG_NEWLINE) {
				flags &= ~FLAG_NEWLINE;
				--plevel;
				return;
			}
			lyxerr << "found newline unexpectedly, array: '" << array << "'\n";
			break;
		}
		
		case LM_TK_BIGSYM:  
		{
			array.push_back(new MathBigopInset(yylval.l->name, yylval.l->id));
			break;
		}
		
		case LM_TK_SYM:
			if (yylval.l->id < 256) {
				MathTextCodes tc = MathIsBOPS(yylval.l->id) ? LM_TC_BOPS: LM_TC_SYMB;
				do_insert(array, yylval.l->id, tc);
			} else 
				do_insert(array, new MathFuncInset(yylval.l->name));
			break;

		case LM_TK_BOP:
			do_insert(array, yylval.i, LM_TC_BOP);
			break;

		case LM_TK_SPACE:
			if (yylval.i >= 0) 
				array.push_back(new MathSpaceInset(yylval.i));
			break;

		case LM_TK_DOTS:
			array.push_back(new MathDotsInset(yylval.l->name, yylval.l->id));
			break;
		
		case LM_TK_CHOOSE:
			handle_frac(array, LM_OT_ATOP);	
			break;

		case LM_TK_STACK:
			handle_frac(array, LM_OT_STACKREL);	
			break;

		case LM_TK_FRAC:
			handle_frac(array, LM_OT_FRAC);	
			break;

		case LM_TK_SQRT:
		{	    
			char c;
			yyis->get(c);
			if (c == '[') {
				MathRootInset * rt = new MathRootInset;
				mathed_parse(rt->cell(0), FLAG_BRACK_END);
				mathed_parse(rt->cell(1), FLAG_BRACE | FLAG_BRACE_LAST);
				array.push_back(rt);
			} else {
				yyis->putback(c);
				MathSqrtInset * sq = new MathSqrtInset;
				mathed_parse(sq->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
				array.push_back(sq);
			}
			break;
		}
		
		case LM_TK_LEFT:
		{
			int ld = yylex();
			if (ld == LM_TK_SYM)
				ld = yylval.l->id;
			else if (ld == LM_TK_STR || ld == LM_TK_BOP || ld == LM_TK_SPECIAL)
				ld = yylval.i;

			MathArray ar;
			mathed_parse(ar, FLAG_RIGHT);

			int rd = yylex();
			if (rd == LM_TK_SYM)
				rd = yylval.l->id;
			else if (rd == LM_TK_STR || rd == LM_TK_BOP || rd == LM_TK_SPECIAL)
				rd = yylval.i;	 

			MathDelimInset * dl = new MathDelimInset(ld, rd);
			dl->setData(ar, 0);
			array.push_back(dl);
			break;
		}
		
		case LM_TK_RIGHT:
			if (flags & FLAG_RIGHT) { 
				--plevel;
				return;
			}
			mathPrintError("Unmatched right delimiter");
//	  panic = true;
			break;
		
		case LM_TK_FONT:
			yy_mtextmode = (yylval.l->id == LM_TC_TEXTRM);
			flags |= (FLAG_BRACE | FLAG_BRACE_FONT);
			break;

		case LM_TK_STY:
		{
			lyxerr << "LM_TK_STY not implemented\n";
			//MathArray tmp = array;
			//MathSizeInset * p = new MathSizeInset(MathStyles(yylval.l->id));
			//array.push_back(p);
			//mathed_parse(p->cell(0), FLAG_BRACE_FONT);
			break; 
		}


		case LM_TK_WIDE:
		{  
			MathDecorationInset * sq = new MathDecorationInset(yylval.l->id);
			mathed_parse(sq->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
			array.push_back(sq);
			break;
		}
		
		case LM_TK_ACCENT:
			setAccent(yylval.l->id);
			break;
			
		case LM_TK_NONUM:
			curr_num = false;
			break;
		
		case LM_TK_PMOD:
		case LM_TK_FUNC:
			if (accent) 
				array.push_back(t, LM_TC_CONST);
			else 
				array.push_back(new MathFuncInset(yylval.l->name));
			break;
		
		case LM_TK_FUNCLIM:
			array.push_back(new MathFuncInset(yylval.l->name, LM_OT_FUNCLIM));
			break;

		case LM_TK_UNDEF: 
			if (MathMacroTable::hasTemplate(yytext)) {
				MathMacro * m = MathMacroTable::cloneTemplate(yytext);
				for (int i = 0; i < m->nargs(); ++i) {
					mathed_parse(m->cell(i), FLAG_BRACE_OPT | FLAG_BRACE_LAST);
					lyxerr << "reading cell " << i << " '" << m->cell(i) << "'\n";
				}
				do_insert(array, m);
			} else
				do_insert(array, new MathFuncInset(yytext, LM_OT_UNDEF));
			break;
		
		case LM_TK_END:
			--plevel;
			return;

		case LM_TK_BEGIN:
		{
			int i = yylval.i;
			MathInsetTypes typ = latex_mathenv[i].typ;

			if (typ == LM_OT_MATRIX) {
				string valign = "\0";
				char rg = LexGetArg(0);
				if (rg == ']') {
					valign = yytext;
					rg = LexGetArg('{');
				}

				string halign = yytext;
				MathArrayInset * mm = new MathArrayInset(halign.size(), 1);
				valign += 'c';
				mm->valign(valign[0]);
				mm->halign(halign);

				mathed_parse_lines(mm, halign.size(), latex_mathenv[i].numbered, false);
				do_insert(array, mm);
				//lyxerr << "read matrix " << *mm << "\n";	
				break;
			} else 
				lyxerr << "unknow math inset " << typ << "\n";	
			break;
		}
	
		case LM_TK_MACRO:
			do_insert(array, MathMacroTable::cloneTemplate(yylval.l->name));
			break;
		
		case LM_TK_LABEL:
		{	
			char const rg = LexGetArg('\0', true);
			if (rg != '}') {
				mathPrintError("Expected '{'");
				// debug info
				lyxerr << "[" << yytext << "]" << endl;
				panic = true;
				break;
			} 
			//lyxerr << " setting label to " << yytext << "\n";
			curr_label = yytext;
			break;
		}
		
		default:
			mathPrintError("Unrecognized token");
			lyxerr << "[" << t << " " << yytext << "]" << endl;
			break;

		} // end of big switch
		
		tprev = t;
		if (panic) {
			lyxerr << " Math Panic, expect problems!" << endl;
			//   Search for the end command. 
			do {
				t = yylex();
			} while (t != LM_TK_END && t);
		} else
			t = yylex();
		
		if (flags & FLAG_BRACE_OPT) {
			flags &= ~FLAG_BRACE_OPT;
			break;
		}
	}
	--plevel;
}


MathInset * mathed_parse(istream & is)
{
	yyis     = &is;
	yylineno = 0;
	return mathed_parse();
}


MathInset * mathed_parse(LyXLex & lex)
{
	yyis     = &lex.getStream();
	yylineno = lex.GetLineNo();

	MathInset * p = mathed_parse();

	// Update line number
	lex.setLineNo(yylineno);

	// reading of end_inset
	while (lex.IsOK()) {
		lex.nextToken();
		if (lex.GetString() == "\\end_inset")
			break;
		lyxerr << "InsetFormula::Read: Garbage before \\end_inset,"
			" or missing \\end_inset!" << endl;
	}

	return p;
}


