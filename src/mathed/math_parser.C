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
#include "math_dotsinset.h"
#include "math_decorationinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_funcinset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_matrixinset.h"
#include "math_rootinset.h"
#include "math_scriptinset.h"
#include "math_sizeinset.h"
#include "math_spaceinset.h"
#include "math_sqrtinset.h"
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


namespace {

void mathed_parse(MathArray & array, unsigned flags);

unsigned char getuchar(std::istream * is)
{
	char c;
	is->get(c);
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
MathTextCodes yyvarcode;



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
		
		if (yyvarcode == LM_TC_TEXTRM && c == ' ') {
			yylval.i = ' ';
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexNewLine) {
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
		} else if (lexcode[c] == LexAlpha) {
			yylval.i = c;
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexBOP) {
			yylval.i = c;
			return LM_TK_BOP;
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
			if (lexcode[c] == LexAlpha) {
				yytext.erase();
				while (lexcode[c] == LexAlpha) {
					yytext += c;
					c = getuchar(yyis);
				}
				if (yyis->good())
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


MathInset * lastUpDownInset(MathArray & array, bool up, bool down)
{
	MathInset * p = array.back_inset();
	if (!p || !p->isUpDownInset()) {
		p = new MathScriptInset(up, down);
		array.push_back(p);
	}
	MathUpDownInset * q = static_cast<MathScriptInset *>(p);
	if (up)
		q->up(true);
	if (down)
		q->down(down);
	return p;
}


MathBigopInset * lastBigopInset(MathArray & array)
{
	MathInset * p = array.back_inset();
	return (p && p->isBigopInset()) ? static_cast<MathBigopInset *>(p) : 0;
}



static bool   curr_num;
static string curr_label;

void mathed_parse_lines(MathInset * inset, int col, bool numbered, bool outmost)
{
	// save global variables
	bool   saved_num   = curr_num;
	string saved_label = curr_label;

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

	// restore global variables
	curr_num   = saved_num;
	curr_label = saved_label;
}


MathInset * mathed_parse()
{
	MathInset * p = 0;
	int t = yylex();

	switch (t) {
		case LM_TK_NEWCOMMAND: {
			string name = lexArg('{').substr(1);
			string arg  = lexArg('[');
			int    narg = arg.empty() ? 0 : atoi(arg.c_str()); 
			p = new MathMacroTemplate(name, narg);
			mathed_parse(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
			//lyxerr[Debug::MATHED] << "LM_TK_NEWCOMMAND: name: "
			//	<< name << " nargs: " << narg << "\n";
			break;
		}

		case LM_TK_BEGIN: {
			int i = yylval.i;
			//lyxerr[Debug::MATHED] << "reading math environment " << i << " "
			//	<< latex_mathenv[i].name << "\n";

			MathInsetTypes typ = latex_mathenv[i].typ;
			p = new MathMatrixInset(typ);
			MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
			switch (typ) {

				case LM_OT_SIMPLE: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label = string();
					mathed_parse(m->cell(0), 0);
					m->numbered(0, curr_num);
					m->label(0, curr_label);
					break;
				}

				case LM_OT_EQUATION: {
					curr_num   = latex_mathenv[i].numbered;
					curr_label = string();
					mathed_parse(m->cell(0), FLAG_END);
					m->numbered(0, curr_num);
					m->label(0, curr_label);
					break;
				}

				case LM_OT_EQNARRAY: {
					mathed_parse_lines(m, 3, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGN: {
					m->halign(lexArg('{'));
					mathed_parse_lines(m, 2, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGNAT: {
					m->halign(lexArg('{'));
					mathed_parse_lines(m, 2, latex_mathenv[i].numbered, true);
					break;
				}

				default: 
					lyxerr[Debug::MATHED] << "1: unknown math environment: " << typ << "\n";
			}

			p->SetName(latex_mathenv[i].basename);

			break;
		}
		
		default:
			lyxerr[Debug::MATHED] << "2 unknown math environment: " << t << "\n";
	}

	return p;
}


void handle_frac(MathArray & array, string const & name)
{
	MathFracInset * p = new MathFracInset(name);
	mathed_parse(p->cell(0), FLAG_ITEM);
	mathed_parse(p->cell(1), FLAG_ITEM);
	array.push_back(p);
}


void mathed_parse(MathArray & array, unsigned flags)
{
	int t = yylex();
	bool panic = false;
	static int plevel = -1;
	yyvarcode = LM_TC_VAR;
	
	int brace = 0;

	++plevel;
	while (t) {
		//lyxerr << "t: " << t << " flags: " << flags << " i: " << yylval.i << " "
		//	<< " plevel: " << plevel << " ";
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
			array.push_back(yylval.i, yyvarcode);
			break;

		case LM_TK_ARGUMENT:
			array.push_back(new MathMacroArgument(yylval.i));
			break;

		case LM_TK_SPECIAL:
			array.push_back(yylval.i, LM_TC_SPECIAL);
			break;

		case LM_TK_STR:
			array.push_back(yylval.i, LM_TC_CONST);
			break;

		case LM_TK_OPEN:
			++brace;
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
			if (flags & FLAG_BRACE_FONT) {
				yyvarcode = LM_TC_VAR;
				flags &= ~FLAG_BRACE_FONT;
				break;
			}
			if (brace == 0 && (flags & FLAG_BRACE_LAST))
				flags |= FLAG_LEAVE;
			else
				array.push_back('}', LM_TC_TEX);
			break;
		
		case '[':
			array.push_back('[', LM_TC_CONST);
			break;

		case ']':
			if (flags & FLAG_BRACK_END)
				flags |= FLAG_LEAVE;
			else 
				array.push_back(']', LM_TC_CONST);
			break;
		
		case '^':
			mathed_parse(lastUpDownInset(array, true, false)->cell(0), FLAG_ITEM);
			break;
		
		case '_':
			mathed_parse(lastUpDownInset(array, false, true)->cell(1), FLAG_ITEM);
			break;
		
		case LM_TK_LIMIT:
		{
			MathBigopInset * p = lastBigopInset(array);
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
			lyxerr[Debug::MATHED] << "found tab unexpectedly, array: '" << array << "'\n";
			break;
		}
		
		case LM_TK_NEWLINE:
		{
			if (flags & FLAG_NEWLINE) {
				flags &= ~FLAG_NEWLINE;
				--plevel;
				return;
			}
			lyxerr[Debug::MATHED] << "found newline unexpectedly, array: '" << array << "'\n";
			break;
		}
		
		case LM_TK_BIGSYM:  
		{
			array.push_back(new MathBigopInset(yylval.l->name, yylval.l->id));
			break;
		}
		
		case LM_TK_SYM:
			if (yylval.l->id < 256) {
				MathTextCodes tc = MathIsBOPS(yylval.l->id) ? LM_TC_BOPS : LM_TC_SYMB;
				array.push_back(yylval.l->id, tc);
			} else 
				array.push_back(new MathFuncInset(yylval.l->name));
			break;

		case LM_TK_BOP:
			array.push_back(yylval.i, LM_TC_BOP);
			break;

		case LM_TK_SPACE:
			if (yylval.i >= 0) 
				array.push_back(new MathSpaceInset(yylval.i));
			break;

		case LM_TK_DOTS:
			array.push_back(new MathDotsInset(yylval.l->name, yylval.l->id));
			break;
		
		case LM_TK_CHOOSE:
			handle_frac(array, "atop");	
			break;

		case LM_TK_STACK:
			handle_frac(array, "stackrel");	
			break;

		case LM_TK_FRAC:
			handle_frac(array, "frac");	
			break;

		case LM_TK_SQRT:
		{
			unsigned char c = getuchar(yyis);
			if (c == '[') {
				array.push_back(new MathRootInset);
				mathed_parse(array.back_inset()->cell(0), FLAG_BRACK_END);
				mathed_parse(array.back_inset()->cell(1), FLAG_ITEM);
			} else {
				yyis->putback(c);
				array.push_back(new MathSqrtInset);
				mathed_parse(array.back_inset()->cell(0), FLAG_ITEM);
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
			//mathed_parse(p->cell(0), FLAG_BRACE_FONT);
			break; 
		}


		case LM_TK_DECORATION:
		{  
			MathDecorationInset * p
				= new MathDecorationInset(yylval.l->name, yylval.l->id);
			mathed_parse(p->cell(0), FLAG_ITEM);
			array.push_back(p);
			break;
		}
			
		case LM_TK_NONUM:
			curr_num = false;
			break;
		
		case LM_TK_PMOD:
		case LM_TK_FUNC:
			array.push_back(new MathFuncInset(yylval.l->name));
			break;
		
		case LM_TK_FUNCLIM:
			array.push_back(new MathFuncInset(yylval.l->name, LM_OT_FUNCLIM));
			break;

		case LM_TK_UNDEF: 
			if (MathMacroTable::hasTemplate(yytext)) {
				MathMacro * m = MathMacroTable::cloneTemplate(yytext);
				for (int i = 0; i < m->nargs(); ++i) 
					mathed_parse(m->cell(i), FLAG_ITEM);
				array.push_back(m);
				m->Metrics(LM_ST_TEXT);
			} else
				array.push_back(new MathFuncInset(yytext, LM_OT_UNDEF));
			break;
		
		case LM_TK_END:
			--plevel;
			return;

		case LM_TK_BEGIN:
		{
			int i = yylval.i;
			MathInsetTypes typ = latex_mathenv[i].typ;

			if (typ == LM_OT_MATRIX) {
				string valign = lexArg('[') + 'c';
				string halign = lexArg('{');
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
			} while (t != LM_TK_END && t);
		} else
			t = yylex();

	}
	--plevel;
}

}

MathInset * mathed_parse(string const & str)
{
	istringstream is(str.c_str());
	return mathed_parse(is);
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
		lyxerr[Debug::MATHED] << "InsetFormula::Read: Garbage before \\end_inset,"
			" or missing \\end_inset!" << endl;
	}

	return p;
}

//]})
