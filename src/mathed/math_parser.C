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
#include "math_rowst.h"
#include "math_iter.h"
#include "math_inset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_root.h"
#include "math_matrixinset.h"
#include "math_accentinset.h"
#include "math_bigopinset.h"
#include "math_funcinset.h"
#include "math_spaceinset.h"
#include "math_dotsinset.h"
#include "math_fracinset.h"
#include "math_deliminset.h"
#include "math_decorationinset.h"
#include "debug.h"
#include "support/lyxlib.h"
#include "mathed/support.h"
#include "boost/array.hpp"

using std::istream;
using std::endl;


extern MathMatrixInset create_multiline(short int type, int cols);

namespace {

enum {
	FLAG_BRACE      = 1,    //  A { needed
	FLAG_BRACE_ARG  = 2,    //  Next { is argument
	FLAG_BRACE_OPT  = 4,    //  Optional {
	FLAG_BRACE_LAST = 8,    //  Last } ends the parsing process
	FLAG_BRACK_ARG  = 16,   //  Optional [
	FLAG_RIGHT      = 32,   //  Next right ends the parsing process
	FLAG_END        = 64,   //  Next end ends the parsing process
	FLAG_BRACE_FONT = 128,  //  Next } closes a font
	FLAG_BRACK_END  = 256   //  Next ] ends the parsing process
};


///
union YYSTYPE {
	///
	unsigned char c;
	///
	char const * s;
	///
	int i;
	///
	latexkeys const * l;
};


YYSTYPE yylval;


MathedInsetTypes mathed_env = LM_OT_MIN;


} // namespace anon


int const latex_mathenv_num = 12;
char const * latex_mathenv[latex_mathenv_num] = { 
	"math", 
	"displaymath", 
	"equation", 
	"eqnarray*",
	"eqnarray",
	"align*",
	"align",
	"alignat*",
	"alignat",
	"multline*",
	"multline",
	"array"
};


char const * latex_special_chars = "#$%&_{}";


namespace {

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
#ifdef WITH_WARNINGS
#warning Replace with string
#endif
//char yytext[256];
boost::array<char, 256> yytext;
int yylineno;
istream * yyis;
bool yy_mtextmode= false;


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
	// unsigned char c;
	// char cc;
	while (yyis->good()) {
		char cc;
		yyis->get(cc);
		unsigned char c = cc;
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
	char const rg =
		(lf == '{') ? '}' :
		((lf == '[') ? ']'
		 : ((lf == '(') ? ')' : 0));
	if (!rg) {
		lyxerr << "Math parse error: unknown bracket '" << lf << "'" << endl;
		return '\0';
	}
	char * p = &yytext[0];
	int bcnt = 1;
	do {
		char cc;
		yyis->get(cc);
		unsigned char c = cc;
		if (c == lf) ++bcnt;
		if (c == rg) --bcnt;
		if ((c > ' ' || (c == ' ' && accept_spaces)) && bcnt > 0)
			*(p++) = c;
	} while (bcnt > 0 && yyis->good() && p - yytext.data() < 255);
	
	*p = '\0';
	return rg;
}


int yylex(void)
{
	static int init_done;
	
	if (!init_done) LexInitCodes();
	
	unsigned char c;
	char cc;
	while (yyis->good()) {
		yyis->get(cc);
		c = cc;
		
		if (yy_mtextmode && c == ' ') {
			yylval.i= ' ';
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexNewLine) {
			++yylineno; 
			continue;
		} else if (lexcode[c] == LexComment) {
			do {
				yyis->get(cc);
				c = cc;
			} while (c != '\n' % yyis->good());  // eat comments
		} else if (lexcode[c] == LexDigit
			   || lexcode[c] == LexOther
			   || lexcode[c] == LexMathSpace) {
			yylval.i = c;
			return LM_TK_STR;
		} else if (lexcode[c] == LexAlpha) {
			yylval.i= c;
			return LM_TK_ALPHA;
		} else if (lexcode[c] == LexBOP) {
			yylval.i= c;
			return LM_TK_BOP;
		} else if (lexcode[c] == LexSelf) {
			return c;
		} else if (lexcode[c] == LexArgument) {
			yyis->get(cc);
			c = cc;
			yylval.i = c - '0';
			return LM_TK_ARGUMENT; 
		} else if (lexcode[c] == LexOpen) {
			return LM_TK_OPEN;
		} else if (lexcode[c] == LexClose) {
			return LM_TK_CLOSE;
		} else if (lexcode[c] == LexESC)   {
			yyis->get(cc);
			c = cc;
			if (c == '\\')	{
				return LM_TK_NEWLINE;
			}
			if (c == '(') {
				yylval.i = LM_OT_MIN;
				return LM_TK_BEGIN;
			}
			if (c == ')') {
				yylval.i = LM_OT_MIN;
				return LM_TK_END;
			}
			if (c == '[') {
				yylval.i = LM_OT_PAR;
				return LM_TK_BEGIN;
			}
			if (c == ']') {
				yylval.i = LM_OT_PAR;
				return LM_TK_END;
			}
			if (
#if 0
				strchr(latex_special_chars, c)
#else
				contains(latex_special_chars, c)
#endif
				) {
				yylval.i = c;
				return LM_TK_SPECIAL;
			} 
			if (lexcode[c] == LexMathSpace) {
				int i;
				for (i = 0; i < 4 && static_cast<int>(c) != latex_mathspace[i][0]; ++i);
				yylval.i = (i < 4) ? i : 0; 
				return LM_TK_SPACE; 
			}
			if (lexcode[c] == LexAlpha || lexcode[c] == LexDigit) {
				char * p = &yytext[0];
				while ((lexcode[c] == LexAlpha || lexcode[c] == LexDigit)
				       && p - yytext.data() < 255) {
					*p = c;
					yyis->get(cc);
					c = cc;
					++p;
				}
				*p = '\0';
				if (yyis->good())
					yyis->putback(c);
				//lyxerr << "reading: '" << yytext.data() << "'\n";
				latexkeys const * l = in_word_set(yytext.data());
				if (l) {
					if (l->token == LM_TK_BEGIN || l->token == LM_TK_END) { 
						int i;
						LexGetArg('{');
//		  for (i = 0; i < 5 && compare(yytext, latex_mathenv[i],
//				strlen(latex_mathenv[i])); ++i);
						
						for (i = 0;
						     i < latex_mathenv_num
							     && compare(yytext.data(), latex_mathenv[i]); ++i);
						yylval.i = i;
					} else if (l->token == LM_TK_SPACE) 
						yylval.i = l->id;
					else
						yylval.l = l;
					return l->token;
				} else { 
					yylval.s = yytext.data();
					return LM_TK_UNDEF;
				}
			}
		}
	}
	return 0;
}


#if 0
int parse_align(char const * hor)
{
	int nc = 0;
	for (char * c = hor; c && *c > ' '; ++c) ++nc;
	return nc;
}
#else
int parse_align(string const & hor)
{
	int nc = 0;
	string::const_iterator cit = hor.begin();
	string::const_iterator end = hor.end();
	for (; cit != end; ++cit)
		if (*cit > ' ') ++nc;
	return nc;
}
#endif

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


MathedInset * doAccent(byte c, MathedTextCodes t)
{
	MathedInset * ac = 0;
	
	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
		  ac = new MathAccentInset(c, t, nestaccent[i]);
		else 
		  ac = new MathAccentInset(ac, nestaccent[i]);
	}
	accent = 0;  // consumed!
	
	return ac;
}


MathedInset * doAccent(MathedInset * p)
{
	MathedInset * ac = 0;
	
	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
		  ac = new MathAccentInset(p, nestaccent[i]);
		else 
		  ac = new MathAccentInset(ac, nestaccent[i]);
	}
	accent = 0;  // consumed!
	
	return ac;
}


void do_insert(MathedIter & it, MathedInset * m, MathedTextCodes t)
{
	if (accent) 
		it.insertInset(doAccent(m), t);
	else
		it.insertInset(m, t);
}


void handle_frac(MathedIter & it, MathParInset * & par, MathedInsetTypes t) 
{
	MathFracInset fc(t);
	MathedArray num;
	mathed_parse(num, par, FLAG_BRACE|FLAG_BRACE_LAST);
	MathedArray den;
	mathed_parse(den, par, FLAG_BRACE|FLAG_BRACE_LAST);
	fc.SetData(num, den);
	it.insertInset(fc.Clone(), LM_TC_ACTIVE_INSET);
}

} // namespace anon


/**
 */
void mathed_parse(MathedArray & array, MathParInset * & par, unsigned flags)
{
	int t = yylex();
	int tprev = 0;
	bool panic = false;
	static int plevel = -1;
	static int size = LM_ST_TEXT;
	MathedTextCodes varcode = LM_TC_VAR;
	MathedInset * binset = 0;
	
	string last_label;              // last label seen
	bool   last_numbered = true;    // have we seen '\nonumber' lately? 

	int brace = 0;
	int acc_brace = 0;
	int acc_braces[8];

	++plevel;
	MathedIter data(&array);
	while (t) {
		//lyxerr << "t: " << t << " par: " << par << " flags: " << flags;
		//lyxerr << "label: '" << last_label << "' ";
		//array.dump(lyxerr);
		//lyxerr << "\n";

		if ((flags & FLAG_BRACE) && t != LM_TK_OPEN) {
			if ((flags & FLAG_BRACK_ARG) && t == '[') {
			} else {
				mathPrintError("Expected {. Maybe you forgot to enclose an argument in {}");
				panic = true;
				break;
			}
		}

		switch (t) {
			
		case LM_TK_ALPHA:
			if (accent) 
				data.insertInset(doAccent(yylval.i, varcode), LM_TC_INSET);
			else
				data.insert(yylval.i, varcode);  //LM_TC_VAR);
			break;

		case LM_TK_ARGUMENT:
		{
			data.insertInset(new MathMacroArgument(yylval.i), LM_TC_INSET);
			break;
		}

		case LM_TK_NEWCOMMAND:
		{
			int na = 0; 
			
			LexGetArg('{');
			string name = &yytext[1];
			
			char const c = yyis->peek();
			if (c == '[') {
				LexGetArg('[');
				na = lyx::atoi(yytext.data());
			}  
			//lyxerr << "LM_TK_NEWCOMMAND: name: " << name << " " << na << endl;
#ifdef WITH_WARNINGS
#warning dirty
#endif
			par->SetName(name);
			par->xo(na); // abuse xo
			flags = FLAG_BRACE|FLAG_BRACE_LAST;

			break;
		}
		
		case LM_TK_SPECIAL:
			data.insert(yylval.i, LM_TC_SPECIAL);
			break;

		case LM_TK_STR:
			if (accent) {
				data.insertInset(doAccent(yylval.i, LM_TC_CONST), LM_TC_INSET);
			} else
				data.insert(yylval.i, LM_TC_CONST);
			break;

		case LM_TK_OPEN:
			++brace;
			if  (accent && tprev == LM_TK_ACCENT) {
				acc_braces[acc_brace++] = brace;
				break;
			}
			if (flags & FLAG_BRACE_OPT) {
				flags &= ~FLAG_BRACE_OPT;
				flags |= FLAG_BRACE;
			}
			
			if (flags & FLAG_BRACE)
				flags &= ~FLAG_BRACE;
			else {
				data.insert('{', LM_TC_TEX);
			}
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
				goto clean_up;
			}
			data.insert('}', LM_TC_TEX);
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
				// if (arg) strcpy(arg, yytext);
			} else
				data.insert('[', LM_TC_CONST);
			break;

		case ']':
			if (flags & FLAG_BRACK_END) {
				--plevel;
				goto clean_up;
			}
			data.insert(']', LM_TC_CONST);
			break;
		
		case '^':
		{  
			MathParInset * p = new MathParInset(size, "", LM_OT_SCRIPT);
			MathedArray ar;
			mathed_parse(ar, par, FLAG_BRACE_OPT|FLAG_BRACE_LAST);
			p->setData(ar);
			// lyxerr << "UP[" << p->GetStyle() << "]" << endl;
			data.insertInset(p, LM_TC_UP);
			break;
		}
		
		case '_':
		{
			MathParInset * p = new MathParInset(size, "",
							    LM_OT_SCRIPT);
			MathedArray ar;
			mathed_parse(ar, par, FLAG_BRACE_OPT|FLAG_BRACE_LAST);
			p->setData(ar);
			data.insertInset(p, LM_TC_DOWN);
			break;
		}
		
		case LM_TK_LIMIT:
			if (binset) {
				binset->SetLimits(bool(yylval.l->id));
				binset = 0;
			}
			break;
		
		case '&':    // Tab
			data.insert('T', LM_TC_TAB);
#ifdef WITH_WARNINGS
#warning look here
#endif
			data.setNumCols(par->GetColumns());
			break;
		
		case LM_TK_NEWLINE:
			//lyxerr << "reading line " << par->getRowSt().size() << "\n";
			if (flags & FLAG_END) {
				if (par->Permit(LMPF_ALLOW_CR)) {
					par->getRowSt().push_back();
					if (last_numbered) {
						//lyxerr << "line " << par->getRowSt().size() << " not numbered\n";
						par->getRowSt().back().setNumbered(false);
						last_numbered = true;
					}
					if (last_label.size()) {
						//lyxerr << "line " << par->getRowSt().size() << " labeled: "
						//	<< last_label << endl;
						par->getRowSt().back().setLabel(last_label);
						last_label.erase();
					}
					data.insert('K', LM_TC_CR);
				} else 
					mathPrintError("Unexpected newline");
			}
			break;

		case LM_TK_BIGSYM:  
		{
			binset = new MathBigopInset(yylval.l->name, yylval.l->id);
			data.insertInset(binset, LM_TC_INSET);	
			break;
		}
		
		case LM_TK_SYM:
			if (yylval.l->id < 256) {
				MathedTextCodes tc = MathIsBOPS(yylval.l->id) ? LM_TC_BOPS: LM_TC_SYMB;
				if (accent) {
					data.insertInset(doAccent(yylval.l->id, tc), LM_TC_INSET);
				} else
					data.insert(yylval.l->id, tc);
			} else {
				MathFuncInset * bg = new MathFuncInset(yylval.l->name);
				if (accent) {
					data.insertInset(doAccent(bg), LM_TC_INSET);
				} else {
#ifdef WITH_WARNINGS
#warning This is suspisious! (Lgb)
#endif
					// it should not take a bool as second arg (Lgb)
					data.insertInset(bg, true);
				}
				
			}
			break;

		case LM_TK_BOP:
			if (accent)
				data.insertInset(doAccent(yylval.i, LM_TC_BOP), LM_TC_INSET);
			else
				data.insert(yylval.i, LM_TC_BOP);
			break;

		case LM_TK_STY:
			par->UserSetSize(yylval.l->id);
			break; 

		case LM_TK_SPACE:
			if (yylval.i >= 0) {
				MathSpaceInset * sp = new MathSpaceInset(yylval.i);
				data.insertInset(sp, LM_TC_INSET);
			}
			break;

		case LM_TK_DOTS:
		{
			MathDotsInset * p = new MathDotsInset(yylval.l->name, yylval.l->id);
			data.insertInset(p, LM_TC_INSET);
			break;
		}
		
		case LM_TK_CHOOSE:
			handle_frac(data, par, LM_OT_ATOP);	
			break;

		case LM_TK_STACK:
			handle_frac(data, par, LM_OT_STACKREL);	
			break;

		case LM_TK_FRAC:
			handle_frac(data, par, LM_OT_FRAC);	
			break;

		case LM_TK_SQRT:
		{	    
			char c;
			yyis->get(c);
			
			if (c == '[') {
				MathRootInset rt(size);

				MathedArray ar1;
				mathed_parse(ar1, par, FLAG_BRACK_END);
				rt.setArgumentIdx(0);
				rt.setData(ar1); // I belive that line is not needed (Lgb)

				MathedArray ar2;
				mathed_parse(ar2, par, FLAG_BRACE|FLAG_BRACE_LAST);

				rt.setArgumentIdx(1);
				rt.setData(ar2); // I belive that this line is not needed (Lgb)

				data.insertInset(rt.Clone(), LM_TC_ACTIVE_INSET);
			} else {
				yyis->putback(c);
				MathSqrtInset rt(size);
				MathedArray ar;
				mathed_parse(ar, par, FLAG_BRACE|FLAG_BRACE_LAST);
				rt.setData(ar); // I belive that this line is not needed (Lgb)
				data.insertInset(rt.Clone(), LM_TC_ACTIVE_INSET);
			}
			break;
		}
		
		case LM_TK_LEFT:
		{
			int lfd = yylex();
			if (lfd == LM_TK_SYM || lfd == LM_TK_STR || lfd == LM_TK_BOP|| lfd == LM_TK_SPECIAL)
				lfd = (lfd == LM_TK_SYM) ? yylval.l->id: yylval.i;
//	 lyxerr << "L[" << lfd << " " << lfd << "]";
			MathedArray ar;
			mathed_parse(ar, par, FLAG_RIGHT);
			int rgd = yylex();
//	 lyxerr << "R[" << rgd << "]";
			if (rgd == LM_TK_SYM || rgd == LM_TK_STR || rgd == LM_TK_BOP || rgd == LM_TK_SPECIAL)
				rgd = (rgd == LM_TK_SYM) ? yylval.l->id: yylval.i;	 
			MathDelimInset * dl = new MathDelimInset(lfd, rgd);
			dl->setData(ar);
			data.insertInset(dl, LM_TC_ACTIVE_INSET);
//	 lyxerr << "RL[" << lfd << " " << rgd << "]";
			break;
		}
		
		case LM_TK_RIGHT:
			if (flags & FLAG_RIGHT) { 
				--plevel;
				goto clean_up;
			}
			mathPrintError("Unmatched right delimiter");
//	  panic = true;
			break;
		
		case LM_TK_FONT:
			varcode = static_cast<MathedTextCodes>(yylval.l->id);
			yy_mtextmode = bool(varcode == LM_TC_TEXTRM);
			flags |= (FLAG_BRACE|FLAG_BRACE_FONT);
			break;

		case LM_TK_WIDE:
		{  
			MathDecorationInset * sq = new MathDecorationInset(yylval.l->id,
									   size);
			MathedArray ar;
			mathed_parse(ar, par, FLAG_BRACE|FLAG_BRACE_LAST);
			sq->setData(ar);
			data.insertInset(sq, LM_TC_ACTIVE_INSET);
			break;
		}
		
		case LM_TK_ACCENT:
			setAccent(yylval.l->id);
			break;
			
		case LM_TK_NONUM:
			//lyxerr << "prepare line " << par->getRowSt().size()
			//	<< " not numbered\n";
			last_numbered = false;
			break;
		
		case LM_TK_PMOD:
		case LM_TK_FUNC:
			if (accent) {
				data.insert(t, LM_TC_CONST);
			} else {
				MathedInset * bg = new MathFuncInset(yylval.l->name); 
				data.insertInset(bg, LM_TC_INSET);
			}
			break;
		
		case LM_TK_FUNCLIM:
			data.insertInset(new MathFuncInset(yylval.l->name, LM_OT_FUNCLIM),
					 LM_TC_INSET);
			break;

		case LM_TK_UNDEF:
		{
			// save this value, yylval.s might get overwritten soon
			const string name = yylval.s;
			//lyxerr << "LM_TK_UNDEF: str = " << name << endl;
			if (MathMacroTable::hasTemplate(name)) {
				MathMacro * m = MathMacroTable::cloneTemplate(name);
				//lyxerr << "Macro: " << m->GetData() << endl;
				for (int i = 0; i < m->nargs(); ++i) {
					MathedArray ar;
					mathed_parse(ar, par, FLAG_BRACE|FLAG_BRACE_LAST);
					m->setData(ar, i);
				}
				do_insert(data, m, m->getTCode());
			} else {
				MathedInset * q = new MathFuncInset(name, LM_OT_UNDEF);
				do_insert(data, q, LM_TC_INSET);
			}
			break;
		}
		
		case LM_TK_END:
			if (mathed_env != yylval.i && yylval.i != LM_OT_MATRIX)
				mathPrintError("Unmatched environment");
			// debug info [made that conditional -JMarc]
			if (lyxerr.debugging(Debug::MATHED))
				lyxerr << "[" << yylval.i << "]" << endl;
			--plevel;

			//if (mt) { // && (flags & FLAG_END)) {
			//	par.setData(array);
			//	array.clear();
			//}
#ifdef WITH_WARNINGS
#warning Look here
#endif
			goto clean_up;

		case LM_TK_BEGIN:
			if (yylval.i == LM_OT_MATRIX) {
				//lyxerr << "###### Reading LM_OT_MATRIX \n";
#if 0
				char ar[120];
				char ar2[8];
				ar[0] = ar2[0] = '\0';
#endif
				char rg = LexGetArg(0);
#if 1
				string ar2;
#endif			
				if (rg == ']') {
#if 0
					strcpy(ar2, yytext.data());
#else
					ar2 = yytext.data();
#endif
					rg = LexGetArg('{');
				}
#if 0
				strcpy(ar, yytext.data());
				int const nc = parse_align(ar);
#else
				string ar(yytext.data());
				int const nc = parse_align(ar);
#endif

				MathParInset * mm = new MathMatrixInset(nc, 0);
				mm->SetAlign(ar2[0], ar);
				MathedArray dat;
				mathed_parse(dat, mm, FLAG_END);
				data.insertInset(mm, LM_TC_ACTIVE_INSET);
				mm->setData(dat);

			} else if (is_eqn_type(yylval.i)) {
				//lyxerr << "###### Reading is_eqn_type \n";
				if (plevel!= 0) {
					mathPrintError("Misplaced environment");
					break;
				}
				
				mathed_env = static_cast<MathedInsetTypes>(yylval.i);
				if (mathed_env != LM_OT_MIN) {
					//lyxerr << "###### Reading mathed_env != LM_OT_MIN \n";
					size = LM_ST_DISPLAY;
					if (is_multiline(mathed_env)) {
						//lyxerr << "###### Reading is_multiline(mathed_env) \n";
						int cols = 1;
						if (is_multicolumn(mathed_env)) {
              //lyxerr << "###### Reading is_multicolumn(mathed_env) \n";
							if (mathed_env != LM_OT_ALIGNAT &&
							    mathed_env != LM_OT_ALIGNATN &&
							    yyis->good()) {
                 //lyxerr << "###### Reading is !align\n";
								char c;
								yyis->get(c);
								if (c != '%')
									lyxerr << "Math parse error: unexpected '"
									       << c << "'" << endl;
							}
							LexGetArg('{');
							cols = strToInt(string(yytext.data()));
						}
#ifdef WITH_WARNINGS
#warning look here
#endif
						//mt = create_multiline(mathed_env, cols);
						//if (mtx) *mtx = mt;

						//MathMatrixInset mat = create_multiline(mathed_env, cols);
						//data.insertInset(mat.Clone(), LM_TC_ACTIVE_INSET);

						par = new MathMatrixInset(create_multiline(mathed_env, cols));
						flags |= FLAG_END;
					}
					par->SetStyle(size);
					par->SetType(mathed_env);
				}
				
				lyxerr[Debug::MATHED] << "MATH BEGIN[" << mathed_env << "]" << endl;
			} else {
				MathMacro * m = MathMacroTable::cloneTemplate(yytext.data());
				data.insertInset(m, m->getTCode());
				MathedArray dat;
				mathed_parse(dat, par, FLAG_END);
			}
			break;
		
		case LM_TK_MACRO:
		{ 
			MathMacro * m = MathMacroTable::cloneTemplate(yylval.l->name);
			do_insert(data, m, m->getTCode());
			break;
		}
		
		case LM_TK_LABEL:
		{	   
			char const rg = LexGetArg('\0', true);
			if (rg != '}') {
				mathPrintError("Expected '{'");
				// debug info
				lyxerr << "[" << yytext.data() << "]" << endl;
				panic = true;
				break;
			} 
			last_label = yytext.data();
			//lyxerr << "prepare line " << par->getRowSt().size()
			//	<< " label: " << last_label << endl;
			break;
		}
		
		default:
			mathPrintError("Unrecognized token");
			// debug info
			lyxerr << "[" << t << " " << yytext.data() << "]" << endl;
			break;
		} // end of switch
		
		tprev = t;
		if (panic) {
			lyxerr << " Math Panic, expect problems!" << endl;
			//   Search for the end command. 
			do {
				t = yylex ();
			} while (t != LM_TK_END && t);
		} else
			t = yylex ();
		
		if ((flags & FLAG_BRACE_OPT)/* && t!= '^' && t!= '_'*/) {
			flags &= ~FLAG_BRACE_OPT;
			break;
		}
	}
	--plevel;

clean_up:

	if (last_numbered == false) {
		//lyxerr << "last line " << par->getRowSt().size() << " not numbered\n";
		if (par->getRowSt().size() == 0)
			par->getRowSt().push_back();
		par->getRowSt().back().setNumbered(false);
	}
	if (last_label.size()) {
		//lyxerr << "last line " << par->getRowSt().size() << " labeled: "
		//	<< last_label << endl;
		if (par->getRowSt().size() == 0)
			par->getRowSt().push_back();
		par->getRowSt().back().setLabel(last_label);
	}
}


void mathed_parser_file(istream & is, int lineno)
{
	yyis = &is;
	yylineno = lineno;
}


int mathed_parser_lineno()
{
	return yylineno;
}
