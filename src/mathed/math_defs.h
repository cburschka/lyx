// -*- C++ -*-
/*
 *  File:        math_defs.h
 *  Purpose:     Math editor definitions 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math paragraph and objects for a WYSIWYG math editor.
 *
 *  Dependencies: Xlib
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 * 
 */

#ifndef MATH_DEFS
#define MATH_DEFS

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#include "LString.h"
#include "debug.h"

//#include "array.h"

class MathedArray;
class Painter;

#ifndef byte
#define byte unsigned char
#endif

///
enum math_align {
	///
	MATH_ALIGN_LEFT = 1,
	///
	MATH_ALIGN_RIGHT = 2,
	///
	MATH_ALIGN_BOTTOM = 4,
	///
	MATH_ALIGN_TOP = 8
};
///
static int const MATH_COLSEP = 8;
///
static int const MATH_ROWSEP = 8;


/// Standard Math Sizes (Math mode styles)
enum MathedStyles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


/** The restrictions of a standard LaTeX math paragraph
  allows to get a small number of text codes (<30) */
enum MathedTextCodes  {
	/// This must be >= 0
	LM_TC_MIN = 0,
	/// Open and Close group
	LM_TC_OPEN,
	///
	LM_TC_CLOSE,
	/// Tabulator
	LM_TC_TAB,
	/// New line
	LM_TC_CR,
	/// Math Inset
	LM_TC_INSET,
	/// Super and sub scripts
	LM_TC_UP,
	///
	LM_TC_DOWN,
	/// Editable Math Inset
	LM_TC_ACTIVE_INSET,
	/// Editable Text Inset
	LM_TC_TEXT_INSET,
	///
	LM_FONT_BEGIN,
	/// Internal code for constants
	LM_TC_CONST,
	/// Internal code for variables
	LM_TC_VAR,
	///
	LM_TC_RM,
	///
	LM_TC_CAL,
	///
	LM_TC_BF,
	///
	LM_TC_SF,
	///
	LM_TC_TT,
	///
	LM_TC_IT,
	///
	LM_TC_TEXTRM,
	/// Math mode TeX characters ",;:{}"
	LM_TC_TEX,
	/// Special characters "{}&#_%"
	LM_TC_SPECIAL,
	/// Internal code for operators
	LM_TC_BOP,
	/// Internal code for symbols
	LM_TC_SYMB,
	///
	LM_TC_BOPS,
	///
	LM_TC_BSYM,
	///
	LM_FONT_END,
	
	/// This must be < 32 
	LM_TC_MAX
};

///
std::ostream & operator<<(std::ostream &, MathedTextCodes mtc);

///
#define LM_TC_NORMAL LM_TC_VAR
 
       
/// Types of lyx-math insets 
enum MathedInsetTypes  {
	///
	LM_OT_MIN = 0,
	/// A simple paragraph
	LM_OT_PAR,
	/// A simple numbered paragraph
	LM_OT_PARN,
	/// A multiline paragraph
	LM_OT_MPAR,
	/// A multiline numbered paragraph
	LM_OT_MPARN,
	///
	LM_OT_ALIGN,
	///
	LM_OT_ALIGNN,
	///
	LM_OT_ALIGNAT,
	///
	LM_OT_ALIGNATN,
	///
	LM_OT_MULTLINE,
	///
	LM_OT_MULTLINEN,
	/// An array
	LM_OT_MATRIX,

	/// A big operator
	LM_OT_BIGOP,
	/// A LaTeX macro
	LM_OT_UNDEF,
	///
	LM_OT_FUNC,
	///
	LM_OT_FUNCLIM,
	///
	LM_OT_SCRIPT,
	///
	LM_OT_SPACE,
	///
	LM_OT_DOTS,
	/// A fraction
	LM_OT_FRAC,
	///
	LM_OT_ATOP,
	///
	LM_OT_STACKREL,
	/// A radical
	LM_OT_SQRT,
	/// A delimiter
	LM_OT_DELIM,
	/// A decoration
	LM_OT_DECO,
	/// An accent
	LM_OT_ACCENT,
	///
	LM_OT_MACRO,
	///
	LM_OT_MACRO_ARG,
	///
	LM_OT_MAX
};

///
enum MathedBinaryTypes {
	///
	LMB_NONE = 0,
	///
	LMB_RELATION,
	///
	LMB_OPERATOR,
	///
	LMB_BOP = (LMB_RELATION | LMB_OPERATOR)
};

class MathParInset;


/// Paragraph permissions
enum MathedParFlag {
	LMPF_BASIC = 0,
	/// If false can use a non-standard size
	LMPF_FIXED_SIZE = 1,
	/// If true can insert newlines 
	LMPF_ALLOW_CR  = 2,
	/// If true can use tabs
	LMPF_ALLOW_TAB = 4,
	/// If true can insert new columns
	LMPF_ALLOW_NEW_COL = 8,
	/// Smaller than current size (frac)
	LMPF_SMALLER = 16,
	/// Script size (subscript, stackrel)
	LMPF_SCRIPT = 32
};





/*************************  Prototypes  **********************************/
/// 
MathedArray * mathed_parse(unsigned flags, MathedArray * data,
			    MathParInset ** mt);
///
void mathed_write(MathParInset *, std::ostream &, int *, bool fragile,
		  string const & label = string());

///
void mathed_parser_file(std::istream &, int);
///
int mathed_parser_lineno();
///
int MathedLookupBOP(short);

/************************ Inline functions ********************************/

///
inline
bool MathIsInset(short x)
{
	return LM_TC_INSET <= x && x <= LM_TC_ACTIVE_INSET;
}

///
inline
bool MathIsFont(short x)
{
	return LM_TC_CONST <= x && x <= LM_TC_BSYM;
}

///
inline
bool MathIsAlphaFont(short x)
{
	return LM_TC_VAR <= x && x <= LM_TC_TEXTRM;
}

///
inline
bool MathIsActive(short x)
{
	return LM_TC_INSET < x && x <= LM_TC_ACTIVE_INSET;
}

///
inline
bool MathIsUp(short x)
{
	return x == LM_TC_UP;
}

///
inline
bool MathIsDown(short x)
{
	return x == LM_TC_DOWN;
}

///
inline
bool MathIsScript(short x)
{
	return x == LM_TC_DOWN || x == LM_TC_UP;
}

///
inline
bool MathIsBOPS(short x)
{
	return MathedLookupBOP(x) > LMB_NONE;
}

///
inline
bool MathIsBinary(short x)
{
    return x == LM_TC_BOP || x == LM_TC_BOPS;
}

///
inline
bool MathIsSymbol(short x) {
    return LM_TC_SYMB <= x && x <= LM_TC_BSYM;
}
     

inline
bool is_eqn_type(short int type)
{
	return type >= LM_OT_MIN && type < LM_OT_MATRIX;
}


inline
bool is_matrix_type(short int type)
{
	return type == LM_OT_MATRIX;
}

inline
bool is_multiline(short int type)
{
	return type >= LM_OT_MPAR && type < LM_OT_MATRIX;
}


inline bool is_ams(short int type)
{
	return type > LM_OT_MPARN && type < LM_OT_MATRIX;
}

inline
bool is_singlely_numbered(short int type)
{
	return type == LM_OT_PARN || type == LM_OT_MULTLINEN;
}

inline
bool is_multi_numbered(short int type)
{
	return type == LM_OT_MPARN || type == LM_OT_ALIGNN
		|| type == LM_OT_ALIGNATN;
}

inline
bool is_numbered(short int type)
{
	return is_singlely_numbered(type) || is_multi_numbered(type);
}

inline
bool is_multicolumn(short int type)
{
	return type == LM_OT_ALIGN || type == LM_OT_ALIGNN
		|| type == LM_OT_ALIGNAT || type == LM_OT_ALIGNATN;
}

#endif
