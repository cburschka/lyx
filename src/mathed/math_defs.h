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
 *   Version: 0.8beta, Math & Lyx project.
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


/// Standard Math Sizes (Math mode styles)
enum MathStyles {
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
enum MathTextCodes  {
	/// This must be >= 0
	LM_TC_MIN = 0,
	/// Open and Close group
	LM_TC_OPEN,
	///
	LM_TC_CLOSE,
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
	/// Internal code for constants  11
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
	/// Math mode TeX characters ",;:{}"  20
	LM_TC_TEX,
	/// Special characters "{}&#_%"
	LM_TC_SPECIAL,
	/// Internal code for operators  22
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


/// Types of lyx-math insets 
enum MathInsetTypes  {
	///
	LM_OT_SIMPLE = 0,
	///
	LM_OT_EQUATION,
	/// 
	LM_OT_EQNARRAY,
	///
	LM_OT_ALIGN,
	///
	LM_OT_ALIGNAT,
	///
	LM_OT_XALIGN,
	///
	LM_OT_XXALIGN,
	///
	LM_OT_MULTLINE,
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
	LM_OT_MAX
};


///
enum MathBinaryTypes {
	///
	LMB_NONE = 0,
	///
	LMB_RELATION,
	///
	LMB_OPERATOR,
	///
	LMB_BOP = (LMB_RELATION | LMB_OPERATOR)
};


/// Paragraph permissions
enum MathParFlag {
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

#endif
