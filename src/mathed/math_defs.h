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


/** The restrictions of a standard LaTeX math paragraph
  allows to get a small number of text codes (<30) */
enum MathTextCodes  {
	/// This must be >= 0
	LM_TC_MIN = 0,
	/// Math Inset
	LM_TC_INSET,

	///
	LM_FONT_BEGIN,
	/// Internal code for variables
	LM_TC_VAR,
	///
	LM_TC_CONST,
	///
	LM_TC_RM,
	///
	LM_TC_CAL,
	///
	LM_TC_EUFRAK,
	///
	LM_TC_BF,
	//
	LM_TC_BB,
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
	/// Internal code when typing greek
	LM_TC_GREEK,
	/// Internal code when typing a single greek character
	LM_TC_GREEK1,
	/// Internal code for symbols
	LM_TC_SYMB,
	/// internal code for symbols that get bigger in displayed math
	LM_TC_BOLDSYMB,
	///
	LM_TC_CMR,
	///
	LM_TC_CMSY,
	///
	LM_TC_CMM,
	///
	LM_TC_CMEX,
	///
	LM_TC_MSA,
	///
	LM_TC_MSB,
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
	LM_OT_XALIGNAT,
	///
	LM_OT_XXALIGNAT,
	///
	LM_OT_MULTLINE,
	///
	LM_OT_GATHER,
	/// An array
	LM_OT_MATRIX,

	/// A LaTeX macro
	LM_OT_UNDEF,
	///
	LM_OT_FUNCLIM,
	///
	LM_OT_MACRO,
	///
	LM_OT_MAX
};

#endif
