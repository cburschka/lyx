// -*- C++ -*-
/*
 *  File:        math_macro.h
 *  Purpose:     Declaration of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Mathed
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.2, Mathed & Lyx project.
 *
 *   This code is under the GNU General Public Licence version 2 or later.
 */
#ifndef MATH_MACRO
#define MATH_MACRO

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include "math_parinset.h"
#include "debug.h"

class MathMacroTemplate;


/// This class contains the data for a macro
class MathMacro : public MathParInset
{
public:
	/// A macro can only be builded from an existing template
	explicit
	MathMacro(MathMacroTemplate *);
	/// or from another macro.
	explicit
	MathMacro(MathMacro *);
	///
	~MathMacro();
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
	///
	MathedInset * Clone();
	///
	void Write(std::ostream &, bool fragile);
	///
	bool setArgumentIdx(int);
	///
	int getArgumentIdx() const;
	///
	int getMaxArgumentIdx() const;
	///
	int GetColumns() const;
	///
	void GetXY(int &, int &) const;
	///
	void SetFocus(int, int);
	///
	MathedArray * GetData();
	///
	MathedRowSt * getRowSt() const;
	///
	void SetData(MathedArray *);
	///
	MathedTextCodes getTCode() const;
	///
	bool Permit(short) const;
    
private:
	///
	MathMacroTemplate * tmplate;
	///
	struct MacroArgumentBase {
		/// Position of the macro
		int x, y;
		///
		MathedRowSt * row;
		///
		MathedArray * array;
		///
		MacroArgumentBase() { x = y = 0;  array = 0; row = 0; }
	};
	std::vector<MacroArgumentBase> args_;
	///
	int idx;
	///
	int nargs;
	///
	MathedTextCodes tcode;
	///
	friend class MathMacroTemplate;
};

///
//typedef MathMacro * MathMacroP;
///
//typedef MathMacroTemplate * MathMacroTemplateP;

#endif
