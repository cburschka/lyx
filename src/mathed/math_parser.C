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
#include <cstdlib>
#include <cctype>

#ifdef __GNUG__
#pragma implementation "math_parser.h"
#endif

#include "math_parser.h"
#include "math_iter.h"
#include "math_inset.h"
#include "math_macro.h"
#include "math_root.h"
#include "debug.h"

enum {
	FLAG_BRACE      = 1,	//  A { needed
	FLAG_BRACE_ARG  = 2,	//  Next { is argument
	FLAG_BRACE_OPT  = 4,	//  Optional {
	FLAG_BRACE_LAST = 8,	//  Last } ends the parsing process
	FLAG_BRACK_ARG  = 16,	//  Optional [
	FLAG_RIGHT	= 32,      //  Next right ends the parsing process
	FLAG_END	= 64,      //  Next end ends the parsing process
	FLAG_BRACE_FONT = 128,	//  Next } closes a font
	FLAG_BRACK_END  = 256   // Next ] ends the parsing process
};

YYSTYPE yylval;


static short mathed_env = LM_EN_INTEXT;

char * mathed_label = 0;

char const * latex_mathenv[] = { 
   "math", 
   "displaymath", 
   "equation", 
   "eqnarray*",
   "eqnarray",
   "array"
};


char const * latex_mathspace[] = {
   "!", ",", ":", ";", "quad", "qquad"
};
   
char const * latex_special_chars = "#$%&_{}";
	    
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

static lexcode_enum lexcode[256];  
static char yytext[256];
static int yylineno;
static istream * yyis;
static bool yy_mtextmode= false;
	    
inline
char * strnew(char const * s)
{
	char * s1 = new char[strlen(s) + 1]; // this leaks when not delete[]'ed
	strcpy(s1, s);
	return s1;
}


static
void mathPrintError(char const * msg) 
{
	lyxerr << "Line ~" << yylineno << ": Math parse error: "
	       << msg << endl;
}


static
void LexInitCodes()
{
   for (int i = 0;  i <= 255; ++i)     {
     if (isalpha(i)) lexcode[i] = LexAlpha;
     else if (isdigit(i)) lexcode[i] = LexDigit;
     else if (isspace(i)) lexcode[i] = LexSpace;
     else lexcode[i] = LexNone;
   }
    
   lexcode['\t'] = lexcode['\f'] = lexcode[' '] = LexSpace;
   lexcode['\n'] = LexNewLine;
   lexcode['%'] = LexComment;
   lexcode['#'] = LexArgument;
   lexcode['+'] = lexcode['-'] = lexcode['*'] = lexcode['/'] = 
   lexcode['<'] = lexcode['>'] = lexcode['='] = LexBOP;
   
   lexcode['!'] = lexcode[','] = lexcode[':'] = lexcode[';'] = LexMathSpace;
   lexcode['('] = lexcode[')'] = lexcode['|'] = lexcode['.'] = lexcode['?'] = LexOther; 
   lexcode['\'']= LexAlpha;
   
   lexcode['['] = lexcode[']'] = lexcode['^'] = lexcode['_'] = 
   lexcode['&'] = LexSelf;  
   
   lexcode['\\'] = LexESC;
   lexcode['{'] = LexOpen;
   lexcode['}'] = LexClose;
}


static
char LexGetArg(char lf, bool accept_spaces= false)
{
	char rg;
	char * p = &yytext[0];
   int bcnt = 1;
   unsigned char c;
   char cc;
   while (yyis->good()) {
      yyis->get(cc);
      c = cc;
      if (c > ' ') {
	 if (!lf) lf = c; else
	 if (c != lf)
		 lyxerr << "Math parse error: unexpected '"
			<< c << "'" << endl;
	 break;
      }
   }
   rg = (lf == '{') ? '}': ((lf == '[') ? ']': ((lf == '(') ? ')': 0));
   if (!rg) {
	   lyxerr << "Math parse error: unknown bracket '"
		  << lf << "'" << endl;
      return '\0';
   } 
   do {
      yyis->get(cc);
      c = cc;
      if (c == lf) ++bcnt;
      if (c == rg) --bcnt;
      if ((c > ' ' || (c == ' ' && accept_spaces)) && bcnt>0) *(p++) = c;
   } while (bcnt > 0 && yyis->good());
   *p = '\0';
   return rg;
}


static
int yylex(void)
{
   static int init_done = 0;
   unsigned char c;
   char cc;
   
   if (!init_done) LexInitCodes();
   
   while (yyis->good()) {
      yyis->get(cc);
      c = cc;
       
      if (yy_mtextmode && c == ' ') {
	  yylval.i= ' ';
	  return LM_TK_ALPHA;
      }
       
       if (lexcode[c] == LexNewLine) {
	   ++yylineno; 
	   continue;
       }
	 
      if (lexcode[c] == LexComment)
	do { yyis->get(cc); c = cc; } while (c != '\n' % yyis->good());  // eat comments
    
      if (lexcode[c] == LexDigit || lexcode[c] == LexOther || lexcode[c] == LexMathSpace) { yylval.i = c; return LM_TK_STR; }
      if (lexcode[c] == LexAlpha) { yylval.i= c; return LM_TK_ALPHA; }
      if (lexcode[c] == LexBOP)   { yylval.i= c; return LM_TK_BOP; }
      if (lexcode[c] == LexSelf)  { return c; }   
      if (lexcode[c] == LexArgument) {
	  yyis->get(cc);
	  c = cc;
	  yylval.i = c - '0';
	  return LM_TK_ARGUMENT; 
      }
      if (lexcode[c] == LexOpen)   { return LM_TK_OPEN; }
      if (lexcode[c] == LexClose)   { return LM_TK_CLOSE; }
      
      if (lexcode[c] == LexESC)   {
	 yyis->get(cc);
	 c = cc;
	 if (c == '\\')	{ return LM_TK_NEWLINE; }
	 if (c == '(')	{ yylval.i = LM_EN_INTEXT; return LM_TK_BEGIN; }
	 if (c == ')')	{ yylval.i = LM_EN_INTEXT; return LM_TK_END; }
	 if (c == '[')	{ yylval.i = LM_EN_DISPLAY; return LM_TK_BEGIN; }
	 if (c == ']')	{ yylval.i = LM_EN_DISPLAY; return LM_TK_END; }
	 if (strchr(latex_special_chars, c)) {
	     yylval.i = c;
	     return LM_TK_SPECIAL;
	 }  
	 if (lexcode[c] == LexMathSpace) {
	    int i;
	    for (i = 0; i < 4 && static_cast<int>(c) != latex_mathspace[i][0]; ++i);
	    yylval.i = (i < 4) ? i: 0; 
	    return LM_TK_SPACE; 
	 }
	 if (lexcode[c] == LexAlpha || lexcode[c] == LexDigit) {
	    char * p = &yytext[0];
	    while (lexcode[c] == LexAlpha || lexcode[c] == LexDigit) {
	       *p = c;
	       yyis->get(cc);
	       c = cc;
	       ++p;
	    }
	    *p = '\0';
	    if (yyis->good()) yyis->putback(c);
	    latexkeys * l = in_word_set (yytext, strlen(yytext));
	    if (l) {
	       if (l->token == LM_TK_BEGIN || l->token == LM_TK_END) { 
		  int i;
		  LexGetArg('{');
//		  for (i = 0; i < 5 && strncmp(yytext, latex_mathenv[i],
//				strlen(latex_mathenv[i])); ++i);
		  
		  for (i = 0; i < 6 && strcmp(yytext, latex_mathenv[i]); ++i);
		  yylval.i = i;
	       } else
	       if (l->token == LM_TK_SPACE) 
		 yylval.i = l->id;
	       else
		 yylval.l = l;
	       return l->token;
	    } else { 
	       yylval.s = yytext;
	       return LM_TK_UNDEF;
	    }
	 }
      }
   }
   return 0;
}


int parse_align(char * hor, char *)
{
   int nc = 0;
   for (char * c = hor; c && *c > ' '; ++c) ++nc;
   return nc;
}


// Accent hacks only for 0.12. Stolen from Cursor.
int accent = 0;
int nestaccent[8];

void setAccent(int ac)
{
	if (ac > 0 && accent < 8) {
		nestaccent[accent++] = ac;
	} else
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


LyxArrayBase * mathed_parse(unsigned flags, LyxArrayBase * array,
			    MathParInset ** mtx)
{
   int t = yylex(), tprev = 0;
   bool panic = false;
   static int plevel = -1;
   static int size = LM_ST_TEXT;
   MathedTextCodes varcode = LM_TC_VAR;
   MathedInset * binset = 0;
   static MathMacroTemplate * macro= 0;
   
   int brace = 0;
   int acc_brace = 0;
   int acc_braces[8];
   MathParInset * mt = (mtx) ? *mtx : 0;//(MathParInset*)0;
    MathedRowSt * crow = (mt) ? mt->getRowSt() : 0;

   ++plevel;
   if (!array) array = new LyxArrayBase;
   MathedIter data(array);
   while (t) {
      if ((flags & FLAG_BRACE) && t != LM_TK_OPEN) {
	 if ((flags & FLAG_BRACK_ARG) && t == '[') {
	 }
	 else {
	     mathPrintError("Expected {. Maybe you forgot to enclose an argument in {}");
	    panic = true;
	    break;
	 }
      }
    MathedInsetTypes fractype = LM_OT_FRAC;
    switch (t) {
    case LM_TK_ALPHA:
      {
	 if (accent) {
	     data.Insert(doAccent(yylval.i, varcode));
	 } else
	    data.Insert (yylval.i, varcode);  //LM_TC_VAR);
	 break;
      }
    case LM_TK_ARGUMENT:
      {
	  if (macro) {
	      data.Insert(macro->getMacroPar(yylval.i-1), LM_TC_INSET);
	  } 
	  break;
      } 
    case LM_TK_NEWCOMMAND:
      {
	  int na = 0; 

	  LexGetArg('{');
	  // This name lives until quitting, for that reason
	  // I didn't care on deleting explicitly. Later I will.
	  char const * name = strnew(&yytext[1]);
	  // ugly trick to be removed soon (lyx3)
	  char c; yyis->get(c);
	  yyis->putback(c);
	  if (c == '[') {
	      LexGetArg('[');
	      na = atoi(yytext);
	  }  
	  macro = new MathMacroTemplate(name, na);
	  flags = FLAG_BRACE|FLAG_BRACE_LAST;
	  *mtx = macro;
	  macro->SetData(array);
	  break;
      }
    case LM_TK_SPECIAL:
      {	  
	  data.Insert (yylval.i, LM_TC_SPECIAL);
	  break;
      }
    case LM_TK_STR:
      {	  
	  if (accent) {
		  data.Insert(doAccent(yylval.i, LM_TC_CONST));
	  } else
	    data.Insert (yylval.i, LM_TC_CONST);
	  break;
      }
    case LM_TK_OPEN:
      {
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
	    data.Insert ('{', LM_TC_TEX);
	 }
	break;
      }
    case LM_TK_CLOSE:
      {
	 --brace;	 
	 if (brace < 0) {
	    mathPrintError("Unmatching braces");
	    panic = true;
	    break;
	 }
	 if (acc_brace && brace == acc_braces[acc_brace-1]-1) {
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
	    return array;
	 } else {
	    data.Insert ('}', LM_TC_TEX);
	 }
	 break;
      }

    case '[':
      {
	 if (flags & FLAG_BRACK_ARG) {
	   flags &= ~FLAG_BRACK_ARG;
	   char rg = LexGetArg('[');
	   if (rg!= ']') {
	      mathPrintError("Expected ']'");
	      panic = true;
	      break;
	   }	   
//	   if (arg) strcpy(arg, yytext);
	} else
	  data.Insert ('[');
	break;
      }
    case ']':
      {
	  if (flags & FLAG_BRACK_END) {
	      --plevel;
	      return array;
	  } else
	    data.Insert (']');
	break;
      }

    case '^':
      {  
	 MathParInset * p = new MathParInset(size, "", LM_OT_SCRIPT);
	 LyxArrayBase * ar = mathed_parse(FLAG_BRACE_OPT|FLAG_BRACE_LAST, 0);
	 p->SetData(ar);
//	 lyxerr << "UP[" << p->GetStyle() << "]" << endl;
	 data.Insert (p, LM_TC_UP);
	 break;
      }
    case '_':
      {
	 MathParInset * p = new MathParInset(size, "", LM_OT_SCRIPT);
	 LyxArrayBase * ar = mathed_parse(FLAG_BRACE_OPT|FLAG_BRACE_LAST, 0);
	 p->SetData(ar);
	 data.Insert (p, LM_TC_DOWN);
	 break;
      }

    case LM_TK_LIMIT:
      {
	 if (binset) {
	    binset->SetLimits(bool(yylval.l->id));
	    binset = 0;
	 }
	 break;
      }
      
    case '&':    // Tab
      {
	 if ((flags & FLAG_END) && mt && data.getCol()<mt->GetColumns()-1) {
	     data.setNumCols(mt->GetColumns());
	     data.Insert('T', LM_TC_TAB);
	 } else 
	    mathPrintError("Unexpected tab");
	 // debug info. [made that conditional -JMarc]
	 if (lyxerr.debugging(Debug::MATHED))
		 lyxerr << data.getCol() << " " << mt->GetColumns() << endl;
	break;
      }
    case LM_TK_NEWLINE:
      {
	  if (mt && (flags & FLAG_END)) {
	      if (mt->Permit(LMPF_ALLOW_CR)) {
		  if (crow) {
			  crow->setNext(new MathedRowSt(mt->GetColumns()+1)); // this leaks
		      crow = crow->getNext();
		  }
		  data.Insert('K', LM_TC_CR);
	      } else 
		mathPrintError("Unexpected newline");
	  }
	  break;
      }
    case LM_TK_BIGSYM:  
      {
	 binset = new MathBigopInset(yylval.l->name, yylval.l->id);
	 data.Insert(binset);	
	 break;
      }
    case LM_TK_SYM:
      {
	 if (yylval.l->id < 256) {
	    MathedTextCodes tc = MathIsBOPS(yylval.l->id) ? LM_TC_BOPS: LM_TC_SYMB;
	    if (accent) {
		data.Insert(doAccent(yylval.l->id, tc));
	    } else
	    data.Insert (yylval.l->id, tc);
	 } else {
	    MathFuncInset * bg = new MathFuncInset(yylval.l->name);
	     if (accent) {
		     data.Insert(doAccent(bg));
	     } else
	     data.Insert(bg, true);	
	 }
	 break;
      }
    case LM_TK_BOP:
      {
	 if (accent) {
		 data.Insert(doAccent(yylval.i, LM_TC_BOP));
	  } else
	    data.Insert (yylval.i, LM_TC_BOP);
	 break;
      }
    case LM_TK_STY:
      {
	  if (mt) {
	      mt->UserSetSize(yylval.l->id);
	  }
	  break; 
      }
    case LM_TK_SPACE:
      {
	 if (yylval.i >= 0) {
	    MathSpaceInset * sp = new MathSpaceInset(yylval.i);
	    data.Insert(sp);
	 }
	 break;
      }	   
    case LM_TK_DOTS:
      {
	 MathDotsInset * p = new MathDotsInset(yylval.l->name, yylval.l->id);
	 data.Insert(p);
	 break;
      }
    case LM_TK_STACK:
       fractype = LM_OT_STACKREL;
    case LM_TK_FRAC:
      {
	 MathFracInset * fc = new MathFracInset(fractype);
	 LyxArrayBase * num = mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST);
	 LyxArrayBase * den = mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST);
	 fc->SetData(num, den);
	 data.Insert(fc, LM_TC_ACTIVE_INSET);
	 break;
      }
    case LM_TK_SQRT:
      {	    
	 MathParInset * rt;
	  
	 char c; yyis->get(c);
	  
	 if (c == '[') {
	     rt = new MathRootInset(size);
	     rt->setArgumentIdx(0);
	     rt->SetData(mathed_parse(FLAG_BRACK_END, 0, &rt));
	     rt->setArgumentIdx(1);
	 } else {
		 yyis->putback(c);
	     rt = new MathSqrtInset(size);
	 }
	 rt->SetData(mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST, 0, &rt));
	 data.Insert(rt, LM_TC_ACTIVE_INSET);
	 break;
      }
       
    case LM_TK_LEFT:
      {
	 int lfd, rgd;
	 lfd = yylex();
	 if (lfd == LM_TK_SYM || lfd == LM_TK_STR || lfd == LM_TK_BOP|| lfd == LM_TK_SPECIAL)
	   lfd = (lfd == LM_TK_SYM) ? yylval.l->id: yylval.i;
//	 lyxerr << "L[" << lfd << " " << lfd << "]";
	 LyxArrayBase * a = mathed_parse(FLAG_RIGHT);
	 rgd = yylex();
//	 lyxerr << "R[" << rgd << "]";
	 if (rgd == LM_TK_SYM || rgd == LM_TK_STR || rgd == LM_TK_BOP || rgd == LM_TK_SPECIAL)
	   rgd = (rgd == LM_TK_SYM) ? yylval.l->id: yylval.i;	 
	 MathDelimInset * dl = new MathDelimInset(lfd, rgd);
	 dl->SetData(a);
	 data.Insert(dl, LM_TC_ACTIVE_INSET);
//	 lyxerr << "RL[" << lfd << " " << rgd << "]";
  	 break;
      }
    case LM_TK_RIGHT:
      {
	 if (flags & FLAG_RIGHT) { 
	    --plevel;
	    return array;
	 } else {
	    mathPrintError("Unmatched right delimiter");
//	    panic = true;
	 }
	 break;
      }

    case LM_TK_FONT:
      {
	 varcode = static_cast<MathedTextCodes>(yylval.l->id);
	  yy_mtextmode = bool(varcode == LM_TC_TEXTRM);
	 flags |= (FLAG_BRACE|FLAG_BRACE_FONT);
	break;
      }
    case LM_TK_WIDE:
      {  
	 MathDecorationInset * sq = new MathDecorationInset(yylval.l->id,
							    size);
	 sq->SetData(mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST));
	 data.Insert(sq, LM_TC_ACTIVE_INSET);
	 break;
      }
      
    case LM_TK_ACCENT: setAccent(yylval.l->id); break;
	  
    case LM_TK_NONUM:
      {
	  if (crow)
	    crow->setNumbered(false);
	  break;
      }

    case LM_TK_PMOD:
    case LM_TK_FUNC:
      {
	  MathedInset * bg = new MathFuncInset(yylval.l->name); 
	  if (accent) {
	      data.Insert(t);
	  } else
	    data.Insert(bg);
	  break;
      }
    case LM_TK_FUNCLIM:
      {
	 data.Insert(new MathFuncInset(yylval.l->name, LM_OT_FUNCLIM));
	 break;
      }
    case LM_TK_UNDEF:
      {
	  
       MathMacro * p = 
	 MathMacroTable::mathMTable.getMacro(yylval.s);
       if (p) {
	   if (accent) 
	     data.Insert(doAccent(p), p->getTCode());
	   else
	     data.Insert(p, p->getTCode());
	   for (int i = 0; p->setArgumentIdx(i); ++i)
	     p->SetData(mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST));
       }
       else {
	   MathedInset * q = new MathFuncInset(yylval.s, LM_OT_UNDEF);
	   if (accent) {
		   data.Insert(doAccent(q));
	   } else {
	       data.Insert(q);
	   }
       }
	 break;
      }
    case LM_TK_END:
      {
         if (mathed_env != yylval.i && yylval.i!= LM_EN_ARRAY)
	   mathPrintError("Unmatched environment");
	 // debug info [made that conditional -JMarc]
	 if (lyxerr.debugging(Debug::MATHED))
		 lyxerr << "[" << yylval.i << "]" << endl;
	 --plevel;
	 if (mt) { // && (flags & FLAG_END)) {
	    mt->SetData(array);
	    array = 0;
	 }
	 return array;
      }
    case LM_TK_BEGIN:
      {
	 if (yylval.i == LM_EN_ARRAY) {
	    char ar[120], ar2[8];
	    ar[0] = ar2[0] = '\0'; 
            char rg = LexGetArg(0);
	    if (rg == ']') {
	       strcpy(ar2, yytext);
	       rg = LexGetArg('{');
	    }
	    strcpy(ar, yytext);
	    int nc = parse_align(ar, ar2);
	    MathParInset * mm = new MathMatrixInset(nc, 0);
	    mm->SetAlign(ar2[0], ar);
       	    data.Insert(mm, LM_TC_ACTIVE_INSET);
            mathed_parse(FLAG_END, mm->GetData(), &mm);
	 } else
	 if (yylval.i >= LM_EN_INTEXT && yylval.i<= LM_EN_EQNARRAY) {
	     if (plevel!= 0) {
		 mathPrintError("Misplaced environment");
		 break;
	     }
	     if (!mt) {
		 mathPrintError("0 paragraph.");
		 panic = true;
	     }
	     
	     mathed_env = yylval.i;
	     if (mathed_env>= LM_EN_DISPLAY) {
		 size = LM_ST_DISPLAY;
		 if (mathed_env>LM_EN_EQUATION) {
		     mt = new MathMatrixInset(3, -1);
		     mt->SetAlign(' ', "rcl");
		     if (mtx) *mtx = mt;
		     flags |= FLAG_END;
//		     data.Insert(' ', LM_TC_TAB);
//		     data.Insert(' ', LM_TC_TAB);
//		     data.Reset();
		 }
		 mt->SetStyle(size);
		 mt->SetType(mathed_env);
		 crow = mt->getRowSt();
	     }
	       	       	       
#ifdef DEBUG
	     lyxerr << "MATH BEGIN[" << mathed_env << "]" << endl;
#endif
	 } else {
//	     lyxerr << "MATHCRO[" << yytext << "]";
	     MathMacro * p = 
	       MathMacroTable::mathMTable.getMacro(yytext);
	     if (p) {
		 data.Insert(p, p->getTCode());
		 p->setArgumentIdx(0);
		 mathed_parse(FLAG_END, p->GetData(), reinterpret_cast<MathParInset**>(&p));
//		 for (int i = 0; p->setArgumentIdx(i); ++i)
//		   p->SetData(mathed_parse(FLAG_BRACE|FLAG_BRACE_LAST));
	     } else 
	       mathPrintError("Unrecognized environment");
	 }
	 break;
      }
       
    case LM_TK_MACRO:
     { 
	  MathedInset * p = 
	    MathMacroTable::mathMTable.getMacro(yylval.l->name);
	 
	  if (p) {
	      if (accent) {
		data.Insert(doAccent(p));
	      } else
		data.Insert(p, static_cast<MathMacro*>(p)->getTCode());
	  }
	  break;
      }
       
     case LM_TK_LABEL:
       {	   
	  char rg = LexGetArg('\0', true);
	  if (rg != '}') {
	     mathPrintError("Expected '{'");
	      // debug info
	     lyxerr << "[" << yytext << "]" << endl;
	      panic = true;
	     break;
	  } 
	  if (crow) {
	      // This is removed by crow's destructor. Bad design? yes, this 
	      // will be changed after 0.12
	      crow->setLabel(strnew(yytext));
	  }
	  else {
		  // where is this math_label free'ed?
	          // Supposedly in ~formula, another bad hack,
	          // give me some time please.
		  mathed_label = strnew(yytext);
	  }
#ifdef DEBUG
	  lyxerr << "Label[" << mathed_label << "]" << endl;
#endif
	  break;
	} 
     default:
       mathPrintError("Unrecognized token");
       // debug info
       lyxerr << "[" << t << " " << yytext << "]" << endl;
       break;
    }
    tprev = t;
    if (panic) {
	    lyxerr << " Math Panic, expect problems!" << endl;
       //   Search for the end command. 
       do t = yylex (); while (t != LM_TK_END && t);
    } else
     t = yylex ();
   
   if ((flags & FLAG_BRACE_OPT)/* && t!= '^' && t!= '_'*/) {
        flags &= ~FLAG_BRACE_OPT;
       //data.Insert (LM_TC_CLOSE);
       break;
    }
   }
   --plevel;
   return array;
}


void mathed_parser_file(istream & is, int lineno)
{
    yyis = &is;
    yylineno = lineno;
    if (!MathMacroTable::built)
	MathMacroTable::mathMTable.builtinMacros();
}


int mathed_parser_lineno()
{
    return yylineno;
}

