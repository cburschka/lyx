// -*- C++ -*-
/*
 *  File:        formula.h
 *  Purpose:     Declaration of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef INSET_FORMULA_MACRO_H 
#define INSET_FORMULA_MACRO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "formula.h"

class MathMacroTemplate;


///
class InsetFormulaMacro: public InsetFormula {
public:
	///
	InsetFormulaMacro();
	///
	InsetFormulaMacro(string name, int na = 0, bool env = false);
	///
	~InsetFormulaMacro();
	///
	int Ascent(LyXFont const & font) const;
	///
	int Descent(LyXFont const & font) const;
	///
	int Width(LyXFont const & font) const;
	///
	void Draw(LyXFont font, LyXScreen & scr, int baseline, float & x);
	///
	void Read(LyXLex & lex);
        ///
	void Write(ostream & os);
	///
	int Latex(ostream & os, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
	///
	Inset * Clone() const;

	/// what appears in the minibuffer when opening
	char const * EditMessage() const {return _("Math macro editor mode");}
	///
	void Edit(int x, int y);
	///
	void InsetUnlock();
	///
	bool LocalDispatch(int, char const *);
protected:
	///
	void UpdateLocal();
private:
	///
        bool opened;
	///
        string name;
	///
        MathMacroTemplate * tmacro;
};

#endif
