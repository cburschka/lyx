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
#include "math_noglyphinset.h"
#include "math_rootinset.h"
#include "math_scopeinset.h"
#include "math_sqrtinset.h"
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


namespace {

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


// These are lexical codes, not semantic
enum lexcode_enum {
	LexNone,
	LexESC,
	LexAlpha,
	LexBOP,         // Binary operators or relations
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


const unsigned char LM_TK_OPEN  = '{';
const unsigned char LM_TK_CLOSE = '}';

enum {
	FLAG_BRACE      = 1 << 0,  //  A { needed              //}
	FLAG_BRACE_LAST = 1 << 1,  //  // { Last } ends the parsing process
	FLAG_RIGHT      = 1 << 2,  //  Next right ends the parsing process
	FLAG_END        = 1 << 3,  //  Next end ends the parsing process
	FLAG_BRACK_END  = 1 << 5,  //  // [ Next ] ends the parsing process
	FLAG_AMPERSAND  = 1 << 6,  //  Next & ends the parsing process
	FLAG_NEWLINE    = 1 << 7,  //  Next \\ ends the parsing process
	FLAG_ITEM       = 1 << 8,  //  read a (possibly braced token)
	FLAG_LEAVE      = 1 << 9,  //  marker for leaving the 
	FLAG_OPTARG     = 1 << 10  //  reads an argument in []
};


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



void lexInit()
{
	for (int i = 0; i <= 255; ++i) {
		if (isdigit(i))
			lexcode[i] = LexOther;
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
	
	lexcode['('] = lexcode[')'] = lexcode['|'] = lexcode['.'] =
		lexcode['?'] = LexOther; 
	
	lexcode['\''] = lexcode['@'] = LexAlpha;
	
	lexcode['['] = lexcode[']'] = lexcode['^'] = lexcode['_'] = 
		lexcode['&'] = LexSelf;  
	
	lexcode['\\'] = LexESC;
	lexcode['{'] = LexOpen;
	lexcode['}'] = LexClose;
}



//
// Helper class for parsing
//


class Parser {
public:
	///
	Parser(LyXLex & lex) : is_(lex.getStream()), lineno_(lex.getLineNo()) {}
	///
	Parser(istream & is) : is_(is), lineno_(0) {}

	///
	MathMacroTemplate * parse_macro();
	///
	MathMatrixInset * parse_normal();
	///
	void parse_into(MathArray & array, unsigned flags);
	///
	int lineno() const { return lineno_; }

private:
	///
	int yylex();
	///
	string lexArg(unsigned char lf, bool accept_spaces = false);
	///
	unsigned char getuchar();
	///
	void error(string const & msg);
	///
	void parse_lines(MathGridInset * p, int col, bool numbered, bool outmost);
	///
	latexkeys const * read_delim();

private:
	///
	istream & is_;
	///
	int lineno_;

	///
	int ival_;
	///
	latexkeys const * lval_;
	///
	string sval_;

	///
	bool   curr_num_;
	///
	string curr_label_;
	///
	string curr_skip_;
};


unsigned char Parser::getuchar()
{
	char c = 0;
	if (!is_.good())
		lyxerr << "The input stream is not well..." << endl;
	is_.get(c);
	return static_cast<unsigned char>(c);
}


string Parser::lexArg(unsigned char lf, bool accept_spaces = false)
{
	string result;
	unsigned char c = 0;
	while (is_.good()) {
		c = getuchar();
		if (!isspace(c))
			break;
	}

	if (c != lf) {
		is_.putback(c);
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
		unsigned char c = getuchar();
		if (c == lf)
			++depth;
		if (c == rg)
			--depth;
		if ((!isspace(c) || (c == ' ' && accept_spaces)) && depth > 0)
			result += c;
	} while (depth > 0 && is_.good());

	return result;
}


int Parser::yylex()
{
	static bool init_done = false;
	
	if (!init_done) {
		lexInit();
		init_done = true;
	}
	
	while (is_.good()) {
		unsigned char c = getuchar();
		//lyxerr << "reading byte: '" << c << "' code: " << lexcode[c] << endl;
		
		if (lexcode[c] == LexNewLine) {
			++lineno_; 
			continue;
		} else if (lexcode[c] == LexComment) {
			do {
				c = getuchar();
			} while (c != '\n' && is_.good());  // eat comments
		} else if (lexcode[c] == LexOther) {
			ival_ = c;
			return LM_TK_STR;
		} else if (lexcode[c] == LexAlpha || lexcode[c] == LexSpace) {
			ival_ = c;
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexBOP) {
			ival_ = c;
			return LM_TK_BOP;
		} else if (lexcode[c] == LexMath) {
			ival_ = 0;
			return LM_TK_MATH;
		} else if (lexcode[c] == LexSelf) {
			return c;
		} else if (lexcode[c] == LexArgument) {
			c = getuchar();
			ival_ = c - '0';
			return LM_TK_ARGUMENT; 
		} else if (lexcode[c] == LexOpen) {
			return LM_TK_OPEN;
		} else if (lexcode[c] == LexClose) {
			return LM_TK_CLOSE;
		} else if (lexcode[c] == LexESC)   {
			c = getuchar();
			//lyxerr << "reading second byte: '" << c << "' code: " << lexcode[c] << endl;
			string s;
			s += c;
			latexkeys const * l = in_word_set(s);
			if (l) {
				//lyxerr << "found key: " << l << endl;
				//lyxerr << "found key name: " << l->name << endl;
				//lyxerr << "found key token: " << l->token << endl;
				lval_ = l;
				ival_ = l->id;
				return l->token;
			}
			if (lexcode[c] == LexAlpha) {
				sval_.erase();
				while (lexcode[c] == LexAlpha && is_.good()) {
					sval_ += c;
					c = getuchar();
				}
				while (lexcode[c] == LexSpace && is_.good()) 
					c = getuchar();
				if (lexcode[c] != LexSpace)
					is_.putback(c);
			
				//lyxerr[Debug::MATHED] << "reading: text '" << sval_ << "'\n";
				//lyxerr << "reading: text '" << sval_ << "'\n";
				latexkeys const * l = in_word_set(sval_);
				if (!l) 
					return LM_TK_UNDEF;

				if (l->token == LM_TK_BEGIN || l->token == LM_TK_END) { 
					string name = lexArg('{');
					int i = 0;
					while (i < latex_mathenv_num && name != latex_mathenv[i].name)
						 ++i;
					ival_ = i;
				} else if (l->token == LM_TK_SPACE) 
					ival_ = l->id;
				else
					lval_ = l;
				return l->token;
			}
		}
	}
	return 0;
}


void Parser::error(string const & msg) 
{
	lyxerr << "Line ~" << lineno_ << ": Math parse error: " << msg << endl;
}


void Parser::parse_lines(MathGridInset * p, int col, bool numbered, bool outmost)
{
	// save global variables
	bool   const saved_num   = curr_num_;
	string const saved_label = curr_label_;

	for (int row = 0; true; ++row) {
		// reset global variables
		curr_num_   = numbered;
		curr_label_.erase();

		// reading a row
		int idx = p->nargs() - p->ncols();
		for (int i = 0; i < col - 1; ++i, ++idx)
			parse_into(p->cell(idx), FLAG_AMPERSAND);
		parse_into(p->cell(idx), FLAG_NEWLINE | FLAG_END);

		if (outmost) {
			MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
			m->numbered(row, curr_num_);
			m->label(row, curr_label_);
			if (curr_skip_.size()) {
				m->vskip(LyXLength(curr_skip_), row);
				curr_skip_.erase();
			}
		}

#ifdef WITH_WARNINGS
#warning Hack!
#endif
		// no newline
		if (ival_ != -1)
			break;

		p->appendRow();
	}

	// restore "global" variables
	curr_num_   = saved_num;
	curr_label_ = saved_label;
}


MathMacroTemplate * Parser::parse_macro()
{
	if (yylex() != LM_TK_NEWCOMMAND) {
		lyxerr << "\\newcommand expected\n";
		return 0;
	}

	string name = lexArg('{').substr(1);
	string arg  = lexArg('[');
	int    narg = arg.empty() ? 0 : atoi(arg.c_str()); 
	MathMacroTemplate * p = new MathMacroTemplate(name, narg);
	parse_into(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
	return p;
}


MathMatrixInset * Parser::parse_normal()
{
	MathMatrixInset * p = 0;
	int t = yylex();

	switch (t) {
		case LM_TK_MATH:
		case LM_TK_BEGIN: {
			int i = ival_;
			lyxerr[Debug::MATHED]
				<< "reading math environment " << i << " "
				<< latex_mathenv[i].name << "\n";

			MathInsetTypes typ = latex_mathenv[i].typ;
			p = new MathMatrixInset(typ);

			switch (typ) {

				case LM_OT_SIMPLE: {
					curr_num_ = latex_mathenv[i].numbered;
					curr_label_.erase();
					parse_into(p->cell(0), 0);
					p->numbered(0, curr_num_);
					p->label(0, curr_label_);
					break;
				}

				case LM_OT_EQUATION: {
					curr_num_ = latex_mathenv[i].numbered;
					curr_label_.erase();
					parse_into(p->cell(0), FLAG_END);
					p->numbered(0, curr_num_);
					p->label(0, curr_label_);
					break;
				}

				case LM_OT_EQNARRAY: {
					parse_lines(p, 3, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGN: {
					p->halign(lexArg('{'));
					parse_lines(p, 2, latex_mathenv[i].numbered, true);
					break;
				}

				case LM_OT_ALIGNAT: {
					p->halign(lexArg('{'));
					parse_lines(p, 2, latex_mathenv[i].numbered, true);
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


latexkeys const * Parser::read_delim()
{
	int ld = yylex();
	//lyxerr << "found symbol: " << ld << "\n";
	latexkeys const * l = in_word_set(".");
	switch (ld) {
		case LM_TK_SYM:
		case LM_TK_NOGLYPH:
		case LM_TK_SPECIAL:
		case LM_TK_BEGIN: {
			l = lval_;
			//lyxerr << "found key 1: '" << l << "'\n";
			//lyxerr << "found key 1: '" << l->name << "'\n";
			break;
		}
		case ']':
		case '[': {
			string s;
			s += ld;
			l = in_word_set(s);
			//lyxerr << "found key 2: '" << l->name << "'\n";
			break;
		}
		case LM_TK_STR: {
			string s;
			s += ival_;
			l = in_word_set(s);
			//lyxerr << "found key 2: '" << l->name << "'\n";
		}
	}
	return l;
}


void Parser::parse_into(MathArray & array, unsigned flags)
{
	MathTextCodes yyvarcode   = LM_TC_VAR;

	int  t      = yylex();
	bool panic  = false;
	int  limits = 0;

	while (t) {
		//lyxerr << "t: " << t << " flags: " << flags << " i: " << ival_
		//	<< " '" << sval_ << "'\n";
		//array.dump(lyxerr);
		//lyxerr << "\n";

		if (flags & FLAG_ITEM) {
			flags &= ~FLAG_ITEM;
			if (t == LM_TK_OPEN) { 
				// skip the brace and collect everything to the next matching
				// closing brace
				t = yylex();
				flags |= FLAG_BRACE_LAST;
			} else {
				// take only this single token
				flags |= FLAG_LEAVE;
			}
		}

		if ((flags & FLAG_BRACE) && t != LM_TK_OPEN) {
			error("Expected {. Maybe you forgot to enclose an argument in {}");
			panic = true;
			break;
		}

		switch (t) {
			
		case LM_TK_ALPHA:
			if (!isspace(ival_) || yyvarcode == LM_TC_TEXTRM)
				array.push_back(new MathCharInset(ival_, yyvarcode));
			break;

		case LM_TK_ARGUMENT: {
			MathMacroArgument * p = new MathMacroArgument(ival_);
			//p->code(yyvarcode);
			array.push_back(p);
			break;
		}

		case LM_TK_SPECIAL:
			array.push_back(new MathCharInset(ival_, LM_TC_SPECIAL));
			break;

		case LM_TK_STR:
			array.push_back(new MathCharInset(ival_, LM_TC_CONST));
			break;

		case LM_TK_OPEN:
			array.push_back(new MathScopeInset);
			parse_into(array.back()->cell(0), FLAG_BRACE_LAST);
			break;

		case LM_TK_CLOSE:
			if (flags & FLAG_BRACE_LAST) {
				flags |= FLAG_LEAVE;
			}
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
			parse_into(
				lastScriptInset(array, true, false, limits)->cell(0), FLAG_ITEM);
			break;
		
		case '_':
			parse_into(
				lastScriptInset(array, false, true, limits)->cell(1), FLAG_ITEM);
			break;
		
		case LM_TK_LIMIT:
			limits = lval_->id;
			//lyxerr << "setting limit to " << limits << "\n";
			break;
		
		case '&':
			if (flags & FLAG_AMPERSAND) {
				flags &= ~FLAG_AMPERSAND;
				return;
			}
			lyxerr[Debug::MATHED]
				<< "found tab unexpectedly, array: '" << array << "'\n";
			break;
		
		case LM_TK_NEWLINE:
		{
			curr_skip_ = lexArg('[');
			if (flags & FLAG_NEWLINE) {
				flags &= ~FLAG_NEWLINE;
				return;
			}
			lyxerr[Debug::MATHED]
				<< "found newline unexpectedly, array: '" << array << "'\n";
			break;
		}
		
		case LM_TK_PROTECT: 
			break;

		case LM_TK_NOGLYPH: 
		case LM_TK_NOGLYPHB: 
			limits = 0;
			array.push_back(new MathNoglyphInset(lval_));
			break;

		case LM_TK_BIGSYM:  
			limits = 0;
			array.push_back(new MathBigopInset(lval_));
			break;

		case LM_TK_FUNCLIM:
			limits = 0;
			array.push_back(new MathFuncLimInset(lval_));
			break;

		case LM_TK_SYM:
			limits = 0;
			array.push_back(new MathSymbolInset(lval_));
			break;

		case LM_TK_BOP:
			array.push_back(new MathCharInset(ival_, LM_TC_BOP));
			break;

		case LM_TK_SPACE:
			if (ival_ >= 0) 
				array.push_back(new MathSpaceInset(ival_));
			break;

		case LM_TK_DOTS:
			array.push_back(new MathDotsInset(lval_));
			break;
		
		case LM_TK_STACK:
		{
			MathStackrelInset * p = new MathStackrelInset;
			parse_into(p->cell(0), FLAG_ITEM);
			parse_into(p->cell(1), FLAG_ITEM);
			array.push_back(p);
			break;
		}

		case LM_TK_FRAC:
		{
			MathFracInset * p = new MathFracInset;
			parse_into(p->cell(0), FLAG_ITEM);
			parse_into(p->cell(1), FLAG_ITEM);
			array.push_back(p);
			break;
		}

		case LM_TK_SQRT:
		{
			unsigned char c = getuchar();
			if (c == '[') {
				array.push_back(new MathRootInset);
				parse_into(array.back()->cell(0), FLAG_BRACK_END);
				parse_into(array.back()->cell(1), FLAG_ITEM);
			} else {
				is_.putback(c);
				array.push_back(new MathSqrtInset);
				parse_into(array.back()->cell(0), FLAG_ITEM);
			}
			break;
		}
		
		case LM_TK_LEFT:
		{
			latexkeys const * l = read_delim();
			MathArray ar;
			parse_into(ar, FLAG_RIGHT);
			latexkeys const * r = read_delim();
			MathDelimInset * dl = new MathDelimInset(l, r);
			dl->cell(0) = ar;
			array.push_back(dl);
			break;
		}
		
		case LM_TK_RIGHT:
			if (flags & FLAG_RIGHT)
				return;
			error("Unmatched right delimiter");
//	  panic = true;
			break;
		
		case LM_TK_FONT:
		{
			MathTextCodes t = static_cast<MathTextCodes>(lval_->id);
			MathArray ar;
			parse_into(ar, FLAG_ITEM);
			for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it)
				(*it)->handleFont(t);
			array.push_back(ar);
			break;
		}

		case LM_TK_OLDFONT:
			yyvarcode = static_cast<MathTextCodes>(lval_->id);
			break;

		case LM_TK_STY:
		{
			lyxerr[Debug::MATHED] << "LM_TK_STY not implemented\n";
			//MathArray tmp = array;
			//MathSizeInset * p = new MathSizeInset(MathStyles(lval_->id));
			//array.push_back(p);
			//parse_into(p->cell(0), FLAG_BRACE_FONT);
			break; 
		}

		case LM_TK_DECORATION:
		{  
			MathDecorationInset * p = new MathDecorationInset(lval_);
			parse_into(p->cell(0), FLAG_ITEM);
			array.push_back(p);
			break;
		}
			
		case LM_TK_NONUM:
			curr_num_ = false;
			break;
		
		case LM_TK_FUNC:
			array.push_back(new MathSymbolInset(lval_));
			break;
		
		case LM_TK_UNDEF: 
			if (MathMacroTable::hasTemplate(sval_)) {
				MathMacro * m = MathMacroTable::cloneTemplate(sval_);
				for (int i = 0; i < m->nargs(); ++i) 
					parse_into(m->cell(i), FLAG_ITEM);
				array.push_back(m);
				m->metrics(LM_ST_TEXT);
			} else
				array.push_back(new MathFuncInset(sval_));
			break;
		
		case LM_TK_MATH:
		case LM_TK_END:
			return;

		case LM_TK_BEGIN:
		{
			int i = ival_;
			MathInsetTypes typ = latex_mathenv[i].typ;

			if (typ == LM_OT_MATRIX) {
				string const valign = lexArg('[') + 'c';
				string const halign = lexArg('{');
				//lyxerr << "valign: '" << valign << "'\n";
				//lyxerr << "halign: '" << halign << "'\n";
				MathArrayInset * m = new MathArrayInset(halign.size(), 1);
				m->valign(valign[0]);
				m->halign(halign);

				parse_lines(m, halign.size(), latex_mathenv[i].numbered, false);
				array.push_back(m);
				//lyxerr << "read matrix " << *m << "\n";	
				break;
			} else 
				lyxerr[Debug::MATHED] << "unknow math inset " << typ << "\n";	
			break;
		}
	
		case LM_TK_MACRO:
			array.push_back(MathMacroTable::cloneTemplate(lval_->name));
			break;
		
		case LM_TK_LABEL:
			curr_label_ = lexArg('{', true);
			break;
		
		default:
			error("Unrecognized token");
			lyxerr[Debug::MATHED] << "[" << t << " " << sval_ << "]" << endl;
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
			} while (is_.good() && t != LM_TK_END && t);
		} else {
			t = yylex();
		}
	}
}


void parse_end(LyXLex & lex, int lineno)
{
	// Update line number
	lex.setLineNo(lineno);

	// reading of end_inset
	while (lex.isOK()) {
		lex.nextToken();
		if (lex.getString() == "\\end_inset")
			break;
		lyxerr[Debug::MATHED] << "InsetFormula::Read: Garbage before \\end_inset,"
			" or missing \\end_inset!" << endl;
	}
}

} // anonymous namespace



MathArray mathed_parse_cell(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	MathArray ar;
	parser.parse_into(ar, 0);
	return ar;
}



MathMacroTemplate * mathed_parse_macro(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_macro();
}

MathMacroTemplate * mathed_parse_macro(istream & is)
{
	Parser parser(is);
	return parser.parse_macro();
}

MathMacroTemplate * mathed_parse_macro(LyXLex & lex)
{
	Parser parser(lex);
	MathMacroTemplate * p = parser.parse_macro();
	parse_end(lex, parser.lineno());
	return p;
}



MathMatrixInset * mathed_parse_normal(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_normal();
}

MathMatrixInset * mathed_parse_normal(istream & is)
{
	Parser parser(is);
	return parser.parse_normal();
}

MathMatrixInset * mathed_parse_normal(LyXLex & lex)
{
	Parser parser(lex);
	MathMatrixInset * p = parser.parse_normal();
	parse_end(lex, parser.lineno());
	return p;
}

