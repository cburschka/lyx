// -*- C++ -*-
/*
 *  File:        math_parser.h
 *  Purpose:     Declaration of parsing utilities for mathed
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

#ifndef MATH_PARSER
#define MATH_PARSER

#ifdef __GNUG__
#pragma interface
#endif

#include "symbol_def.h"


#define LM_TK_OPEN '{'

#define LM_TK_CLOSE '}'


enum MathTokenEnum
{
   LM_TK_BOP = 256,
     LM_TK_ALPHA,
     LM_TK_STR,
     LM_TK_SYM,
     LM_TK_FRAC,
     LM_TK_SQRT,
     LM_TK_BEGIN,
     LM_TK_END,
     LM_TK_NEWLINE,
     LM_TK_UNDEF,
     LM_TK_FONT,
     LM_TK_LEFT,
     LM_TK_RIGHT,
     LM_TK_ACCENT,
     LM_TK_WIDE,
     LM_TK_FUNC,
     LM_TK_FUNCLIM,
     LM_TK_BIGSYM,
     LM_TK_LABEL,
     LM_TK_NONUM,
     LM_TK_SPACE,
     LM_TK_DOTS,
     LM_TK_LIMIT,
     LM_TK_STY,
     LM_TK_PMOD,
     LM_TK_BMOD,
     LM_TK_MACRO,
     LM_TK_SPECIAL,
     LM_TK_ARGUMENT, 
     LM_TK_NEWCOMMAND,
     LM_TK_STACK
};


struct latexkeys {
	char const * name;
	short token;
	int id;
};


latexkeys *
in_word_set (register char const * str, register int len);


latexkeys * lm_get_key(int index);


latexkeys * lm_get_key_by_id(int id, short tc = LM_TK_SYM);


typedef union{

unsigned char c;

char * s;

int i;

latexkeys * l;
} YYSTYPE;

extern YYSTYPE yylval;

#endif
