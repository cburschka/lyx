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


/// Types of lyx-math insets
enum MathInsetTypes  {
	///
	LM_OT_NONE = 0,
	///
	LM_OT_SIMPLE,
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
	/// an array
	LM_OT_MATRIX,

	/// a LaTeX macro
	LM_OT_MACRO
};

#endif
