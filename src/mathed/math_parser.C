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

// {[(

#include <config.h>

#include <cctype>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_parser.h"
#include "array.h"
#include "math_inset.h"
#include "math_arrayinset.h"
#include "math_bigopinset.h"
#include "math_charinset.h"
#include "math_dotsinset.h"
#include "math_decorationinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_funcinset.h"
#include "math_funcliminset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_matrixinset.h"
#include "math_rootinset.h"
#include "math_scriptinset.h"
#include "math_sizeinset.h"
#include "math_spaceinset.h"
#include "math_sqrtinset.h"
#include "math_stackrelinset.h"
#include "math_symbolinset.h"
#include "debug.h"
#include "mathed/support.h"
#include "lyxlex.h"
#include "support/lstrings.h"

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
	LexMath,
	LexSelf
};

lexcode_enum lexcode[256];  


namespace {

void mathed_parse_into(MathArray & array, unsigned flags);

unsigned char getuchar(std::istream * is)
{
	char c = 0;
	is->get(c);
	if (!is->good())
		lyxerr << "The input stream is not well..." << endl;
	
	return static_cast<unsigned char>(c);
}

const unsigned char LM_TK_OPEN  = '{';
const unsigned char LM_TK_CLOSE = '}';

enum {
	FLAG_BRACE      = 1 << 0,  //  A { needed              //}
	FLAG_BRACE_LAST = 1 << 1,  //  // { Last } ends the parsing process
	FLAG_RIGHT      = 1 << 2,  //  Next right ends the parsing process
	FLAG_END        = 1 << 3,  //  Next end ends the parsing process
	FLAG_BRACE_FONT = 1 << 4,  //  // { Next } closes a font
	FLAG_BRACK_END  = 1 << 5,  //  // [ Next ] ends the parsing process
	FLAG_AMPERSAND  = 1 << 6,  //  Next & ends the parsing process
	FLAG_NEWLINE    = 1 << 7,  //  Next \\ ends the parsing process
	FLAG_ITEM       = 1 << 8,  //  read a (possibly braced token)
	FLAG_LEAVE      = 1 << 9,  //  marker for leaving the 
	FLAG_OPTARG     = 1 << 10  //  reads an argument in []
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
	//lyxerr[Debug::MATHED] << "Line ~" << yylineno << ": Math parse error: " << msg << endl;
	lyxerr << "Line ~" << yylineno << ": Math parse error: " << msg << endl;
}


void lexInit()
{
	for (int i = 0; i <= 255; ++i) {
		if (isdigit(i))
			lexcode[i] = LexDigit;
		else if (isspace(i))
			lexcode[i] = LexSpace;
		else
			lexcode[i] = LexAlpha;
	}
	
	lexcode['\t'] = lexcode['\f'] = lexcode[' '] = LexSpace;
	lexcode['\n'] = LexNewLine;
	lexcode['%'] = LexComment;
	lexcode['#'] = LexArgument;
	lexcode['$'] = LexMath;
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


string lexArg(unsigned char lf, bool accept_spaces = false)
{
	string result;
	unsigned char c = 0;
	while (yyis->good()) {
		c = getuchar(yyis);
		if (!isspace(c))
			break;
	}

	if (c != lf) {
		yyis->putback(c);
		return result;
	}
		
	unsigned char rg = 0;
	if (lf == '{') rg = '}';
	if (lf == '[') rg = ']';
	if (lf == '(') rg = ')';
	if (!rg) {
		lyxerr[Debug::MATHED] << "Math parse error: unknown bracket '"
			<< lf << "'" << endl;
		return result;
	}

	int depth = 1;
	do {
		unsigned char c = getuchar(yyis);
		if (c == lf)
			++depth;
		if (c == rg)
			--depth;
		if ((!isspace(c) || (c == ' ' && accept_spaces)) && depth > 0)
			result += c;
	} while (depth > 0 && yyis->good());

	return result;
}


int yylex()
{
	static bool init_done = false;
	
	if (!init_done) {
		lexInit();
		init_done = true;
	}
	
	while (yyis->good()) {
		unsigned char c = getuchar(yyis);
		//lyxerr << "reading byte: '" << c << "' code: " << lexcode[c] << endl;
		
		if (lexcode[c] == LexNewLine) {
			++yylineno; 
			continue;
		} else if (lexcode[c] == LexComment) {
			do {
				c = getuchar(yyis);
			} while (c != '\n' && yyis->good());  // eat comments
		} else if (lexcode[c] == LexDigit
			   || lexcode[c] == LexOther
			   || lexcode[c] == LexMathSpace) {
			yylval.i = c;
			return LM_TK_STR;
		} else if (lexcode[c] == LexAlpha || lexcode[c] == LexSpace) {
			yylval.i = c;
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexBOP) {
			yylval.i = c;
			return LM_TK_BOP;
		} else if (lexcode[c] == LexMath) {
			yylval.i = 0;
			return LM_TK_MATH;
		} else if (lexcode[c] == LexSelf) {
			return c;
		} else if (lexcode[c] == LexArgument) {
			c = getuchar(yyis);
			yylval.i = c - '0';
			return LM_TK_ARGUMENT; 
		} else if (lexcode[c] == LexOpen) {
			return LM_TK_OPEN;
		} else if (lexcode[c] == LexClose) {
			return LM_TK_CLOSE;
		} else if (lexcode[c] == LexESC)   {
			c = getuchar(yyis);
			string s;
			s += c;
			latexkeys const * l = in_word_set(s);
			if (l) {
				//lyxerr << "found special token for '" << l->name
				//	<< "' : " << l->id << " \n";
				yylval.i = l->id;
				return l->token;
			}
			if (lexcode[c] == LexAlpha) {
				yytext.erase();
				while (lexcode[c] == LexAlpha && yyis->good()) {
					yytext += c;
					c = getuchar(yyis);
				}
				while (lexcode[c] == LexSpace && yyis->good()) 
					c = getuchar(yyis);
				if (lexcode[c] != LexSpace)
					yyis->putback(c);
			
				//lyxerr[Debug::MATHED] << "reading: text '" << yytext << "'\n";
				latexkeys const * l = in_word_set(yytext);
				if (!l) 
					return LM_TK_UNDEF;

				if (l->token == LM_TK_BEGIN || l->token == LM_TK_END) { 
					string name = lexArg('{');
					int i = 0;
					while (i < latex_mathenv_num && name != latex_mathenv[i].name)
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


MathScriptInset * prevScriptInset(MathArray const & array)
{
	MathInset * p = array.back();
	return (p && p->isScriptInset()) ? static_cast<MathScriptInset *>(p) : 0;
}


MathInset * lastScriptInset(MathArray & array, bool up, bool down, int limits)
{
	MathScriptInset * p = prevScriptInset(array);
	if (!p) {
		MathInset * b = array.back();
		if (b && b->isScriptable()) {
			p = new MathScriptInset(up, down, b->clone());
			array.pop_back();	
		} else {
			p = new MathScriptInset(up, down);
		}
		array.push_back(p);
	}
	if (up)
		p->up(true);
	if (down)
		p->down(down);
	if (limits)
		p->limits(limits);
	return p;
}


static bool   curr_num;
static string curr_label;

void mathed_parse_lines(MathGridInset * p, int col, bool numbered, bool outmost)
{
	// save global variables
	bool   const saved_num   = curr_num;
	string const saved_label = curr_label;

	for (int row = 0; true; ++row) {
		// reset global variables
		curr_num   = numbered;
		curr_label.erase();

		// reading a row
		int idx = p->nargs() - p->ncols();
		for (int i = 0; i < col - 1; ++i, ++idx)
			mathed_parse_into(p->cell(idx), FLAG_AMPERSAND);
		mathed_parse_into(p->cell(idx), FLAG_NEWLINE | FLAG_END);

		if (outmost) {
			MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
			m->numbered(row, curr_num);
			m->label(row, curr_label);
		}

#ifdef WITH_WARNINGS
#warning Hack!
#endif
		// no newline
		if (yylval.i != -1)
			break;

		p->appendRow();
	}

	// restore global variables
	curr_num   = saved_num;
	curr_label = saved_label;
}


MathMacroTemplate * mathed_parse_macro()
{
	if (yylex() != LM_TK_NEWCOMMAND) {
		lyxerr << "\\newcommand expected\n";
		return 0;
	}

	string name = lexArg('{').substr(1);
	string arg  = lexArg('[');
	int    narg = arg.empty() ? 0 : atoi(arg.c_str()); 
	MathMacroTemplate * p = new MathMacroTemplate(name, narg);
	mathed_parse_into(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
	return p;
}


MathMatrixInset * mathed_parse_normal()
{
	MathMatrixInset * p = 0;
	int t = yylex();

	switch (t) {
		case LM_TK_MATH:
		case LM_TK_BEGIN: {
			int i = yylval.i;
			lyxerr[Debug::MATHED]
				<< "reading math environment " << i << " "
				<< latex_mathenv[i].name << "\n";

			MathInsetTypes typ = latex_mathenv[i].typ;
			p = new MathMatrixInset(typ);

			switch (typ) {

				case LM_OT_SIMPLE: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label.erase();
					mathed_parse_into(p->cell(0), 0);
					p->numbered(0, curr_num);
					p->label(0, curr_label);
					break;
				}

				case LM_OT_EQUATION: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label.erase();
					mathed_parse_into(p->cell(0), FLAG_END);
					p->numbered(0, curr_num);
					p->label(0, curr_label);
					break;
				}

				case LM_OT_EQNARRAY: {
					mathed_parse_lines(p, 3, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGN: {
					p->halign(lexArg('{'));
					mathed_parse_lines(p, 2, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGNAT: {
					p->halign(lexArg('{'));
					mathed_parse_lines(p, 2, latex_mathenv[i].numbered, true);
					break;
				}

				default: 
					lyxerr[Debug::MATHED]
						<< "1: unknown math environment: " << typ << "\n";
			}

			break;
		}
		
		default:
			lyxerr[Debug::MATHED]
				<< "2 unknown math environment: " << t << "\n";
	}

	return p;
}


void mathed_parse_into(MathArray & array, unsigned flags)
{
	static int plevel = -1;

	++plevel;
	MathTextCodes yyvarcode   = LM_TC_VAR;

	int  t      = yylex();
	bool panic  = false;
	int  brace  = 0;
	int  limits = 0;

	while (t) {
		//lyxerr << "t: " << t << " flags: " << flags << " i: " << yylval.i
		//	<< " '" << yytext << "'\n";
		//array.dump(lyxerr);
		//lyxerr << "\n";

		if (flags & FLAG_ITEM) {
			flags &= ~FLAG_ITEM;
			if (t == LM_TK_OPEN) { 
				// skip the brace and regard everything to the next matching
				// closing brace
				t = yylex();
				++brace;
				flags |= FLAG_BRACE_LAST;
			} else {
				// regard only this single token
				flags |= FLAG_LEAVE;
			}
		}

		if ((flags & FLAG_BRACE) && t != LM_TK_OPEN) {
			mathPrintError(
				"Expected {. Maybe you forgot to enclose an argument in {}");
			panic = true;
			break;
		}

		switch (t) {
			
		case LM_TK_ALPHA:
			if (!isspace(yylval.i) || yyvarcode == LM_TC_TEXTRM)
				array.push_back(new MathCharInset(yylval.i, yyvarcode));
			break;

		case LM_TK_ARGUMENT: {
			MathMacroArgument * p = new MathMacroArgument(yylval.i);
			p->code(yyvarcode);
			array.push_back(p);
			break;
		}

		case LM_TK_SPECIAL:
			array.push_back(new MathCharInset(yylval.i, LM_TC_SPECIAL));
			break;

		case LM_TK_STR:
			array.push_back(new MathCharInset(yylval.i, LM_TC_CONST));
			break;

		case LM_TK_OPEN:
			++brace;
			if (flags & FLAG_BRACE)
				flags &= ~FLAG_BRACE;
			else 
				array.push_back(new MathCharInset('{', LM_TC_TEX));
			break;

		case LM_TK_CLOSE:
			--brace;
			if (brace < 0) {
				mathPrintError("Unmatching braces");
				panic = true;
				break;
			}
			if (flags & FLAG_BRACE_FONT) {
				yyvarcode = LM_TC_VAR;
				flags &= ~FLAG_BRACE_FONT;
				break;
			}
			if (brace == 0 && (flags & FLAG_BRACE_LAST))
				flags |= FLAG_LEAVE;
			else
				array.push_back(new MathCharInset('}', LM_TC_TEX));
			break;
		
		case '[':
			array.push_back(new MathCharInset('[', LM_TC_CONST));
			break;

		case ']':
			if (flags & FLAG_BRACK_END)
				flags |= FLAG_LEAVE;
			else 
				array.push_back(new MathCharInset(']', LM_TC_CONST));
			break;
		
		case '^':
			mathed_parse_into(
				lastScriptInset(array, true, false, limits)->cell(0), FLAG_ITEM);
			break;
		
		case '_':
			mathed_parse_into(
				lastScriptInset(array, false, true, limits)->cell(1), FLAG_ITEM);
			break;
		
		case LM_TK_LIMIT:
			limits = yylval.l->id;
			//lyxerr << "setting limit to " << limits << "\n";
			break;
		
		case '&':
			if (flags & FLAG_AMPERSAND) {
				flags &= ~FLAG_AMPERSAND;
				--plevel;
				return;
			}
			lyxerr[Debug::MATHED]
				<< "found tab unexpectedly, array: '" << array << "'\n";
			break;
		
		case LM_TK_NEWLINE:
			if (flags & FLAG_NEWLINE) {
				flags &= ~FLAG_NEWLINE;
				--plevel;
				return;
			}
			lyxerr[Debug::MATHED]
				<< "found newline unexpectedly, array: '" << array << "'\n";
			break;
		
		case LM_TK_PROTECT: 
			break;

		case LM_TK_BIGSYM:  
			limits = 0;
			array.push_back(new MathBigopInset(yylval.l));
			break;

		case LM_TK_FUNCLIM:
			limits = 0;
			array.push_back(new MathFuncLimInset(yylval.l));
			break;

		case LM_TK_SYM:
			limits = 0;
			array.push_back(new MathSymbolInset(yylval.l));
			break;

		case LM_TK_BOP:
			array.push_back(new MathCharInset(yylval.i, LM_TC_BOP));
			break;

		case LM_TK_SPACE:
			if (yylval.i >= 0) 
				array.push_back(new MathSpaceInset(yylval.i));
			break;

		case LM_TK_DOTS:
			array.push_back(new MathDotsInset(yylval.l));
			break;
		
		case LM_TK_STACK:
		{
			MathStackrelInset * p = new MathStackrelInset;
			mathed_parse_into(p->cell(0), FLAG_ITEM);
			mathed_parse_into(p->cell(1), FLAG_ITEM);
			array.push_back(p);
			break;
		}

		case LM_TK_FRAC:
		{
			MathFracInset * p = new MathFracInset;
			mathed_parse_into(p->cell(0), FLAG_ITEM);
			mathed_parse_into(p->cell(1), FLAG_ITEM);
			array.push_back(p);
			break;
		}

		case LM_TK_SQRT:
		{
			unsigned char c = getuchar(yyis);
			if (c == '[') {
				array.push_back(new MathRootInset);
				mathed_parse_into(array.back()->cell(0), FLAG_BRACK_END);
				mathed_parse_into(array.back()->cell(1), FLAG_ITEM);
			} else {
				yyis->putback(c);
				array.push_back(new MathSqrtInset);
				mathed_parse_into(array.back()->cell(0), FLAG_ITEM);
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
			mathed_parse_into(ar, FLAG_RIGHT);

			int rd = yylex();
			if (rd == LM_TK_SYM)
				rd = yylval.l->id;
			else if (rd == LM_TK_STR || rd == LM_TK_BOP || rd == LM_TK_SPECIAL)
				rd = yylval.i;	 

			MathDelimInset * dl = new MathDelimInset(ld, rd);
			dl->cell(0) = ar;
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
			yyvarcode = static_cast<MathTextCodes>(yylval.l->id);
			flags |= (FLAG_BRACE | FLAG_BRACE_FONT);
			break;

		case LM_TK_STY:
		{
			lyxerr[Debug::MATHED] << "LM_TK_STY not implemented\n";
			//MathArray tmp = array;
			//MathSizeInset * p = new MathSizeInset(MathStyles(yylval.l->id));
			//array.push_back(p);
			//mathed_parse_into(p->cell(0), FLAG_BRACE_FONT);
			break; 
		}


		case LM_TK_DECORATION:
		{  
			MathDecorationInset * p = new MathDecorationInset(yylval.l);
			mathed_parse_into(p->cell(0), FLAG_ITEM);
			array.push_back(p);
			break;
		}
			
		case LM_TK_NONUM:
			curr_num = false;
			break;
		
		case LM_TK_FUNC:
			array.push_back(new MathSymbolInset(yylval.l));
			break;
		
		case LM_TK_UNDEF: 
			if (MathMacroTable::hasTemplate(yytext)) {
				MathMacro * m = MathMacroTable::cloneTemplate(yytext);
				for (int i = 0; i < m->nargs(); ++i) 
					mathed_parse_into(m->cell(i), FLAG_ITEM);
				array.push_back(m);
				m->metrics(LM_ST_TEXT);
			} else
				array.push_back(new MathFuncInset(yytext));
			break;
		
		case LM_TK_MATH:
		case LM_TK_END:
			--plevel;
			return;

		case LM_TK_BEGIN:
		{
			int i = yylval.i;
			MathInsetTypes typ = latex_mathenv[i].typ;

			if (typ == LM_OT_MATRIX) {
				string const valign = lexArg('[') + 'c';
				string const halign = lexArg('{');
				//lyxerr << "valign: '" << valign << "'\n";
				//lyxerr << "halign: '" << halign << "'\n";
				MathArrayInset * m = new MathArrayInset(halign.size(), 1);
				m->valign(valign[0]);
				m->halign(halign);

				mathed_parse_lines(m, halign.size(), latex_mathenv[i].numbered, false);
				array.push_back(m);
				//lyxerr << "read matrix " << *m << "\n";	
				break;
			} else 
				lyxerr[Debug::MATHED] << "unknow math inset " << typ << "\n";	
			break;
		}
	
		case LM_TK_MACRO:
			array.push_back(MathMacroTable::cloneTemplate(yylval.l->name));
			break;
		
		case LM_TK_LABEL:
			curr_label = lexArg('{', true);
			break;
		
		default:
			mathPrintError("Unrecognized token");
			lyxerr[Debug::MATHED] << "[" << t << " " << yytext << "]" << endl;
			break;

		} // end of big switch

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}

		if (panic) {
			lyxerr << " Math Panic, expect problems!" << endl;
			//   Search for the end command. 
			do {
				t = yylex();
			} while (yyis->good() && t != LM_TK_END && t);
		} else {
			t = yylex();
		}
	}
	--plevel;
}

}


MathArray mathed_parse_cell(string const & str)
{
	istringstream is(str.c_str());
	yyis     = &is;
	yylineno = 0;
	MathArray ar;
	mathed_parse_into(ar, 0);
	return ar;
}


MathMacroTemplate * mathed_parse_macro(string const & str)
{
	istringstream is(str.c_str());
	return mathed_parse_macro(is);
}


MathMatrixInset * mathed_parse_normal(string const & str)
{
	istringstream is(str.c_str());
	return mathed_parse_normal(is);
}



MathMatrixInset * mathed_parse_normal(istream & is)
{
	yyis     = &is;
	yylineno = 0;
	return mathed_parse_normal();
}


MathMacroTemplate * mathed_parse_macro(istream & is)
{
	yyis     = &is;
	yylineno = 0;
	return mathed_parse_macro();
}



MathMatrixInset * mathed_parse_normal(LyXLex & lex)
{
	yyis     = &lex.getStream();
	yylineno = lex.getLineNo();

	MathMatrixInset * p = mathed_parse_normal();

	// Update line number
	lex.setLineNo(yylineno);

	// reading of end_inset
	while (lex.isOK()) {
		lex.nextToken();
		if (lex.getString() == "\\end_inset")
			break;
		lyxerr[Debug::MATHED] << "InsetFormula::Read: Garbage before \\end_inset,"
			" or missing \\end_inset!" << endl;
	}

	return p;
}

MathMacroTemplate * mathed_parse_macro(LyXLex & lex)
{
	yyis     = &lex.getStream();
	yylineno = lex.getLineNo();

	MathMacroTemplate * p = mathed_parse_macro();

	// Update line number
	lex.setLineNo(yylineno);

	// reading of end_inset
	while (lex.isOK()) {
		lex.nextToken();
		if (lex.getString() == "\\end_inset")
			break;
		lyxerr[Debug::MATHED] << "InsetFormula::Read: Garbage before \\end_inset,"
			" or missing \\end_inset!" << endl;
	}

	return p;
}
//]})
