// -*- C++ -*-
/*
 *  File:        formula.h
 *  Purpose:     Declaration of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
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
	explicit
	InsetFormulaMacro(string name, int na = 0, bool env = false);
	///
	~InsetFormulaMacro();
	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(BufferView *,LyXFont const &, int, float &, bool) const;
	///
	void Read(Buffer const *, LyXLex & lex);
        ///
	void Write(Buffer const *, std::ostream & os) const;
	///
	int Latex(Buffer const *, std::ostream & os, bool fragile,
		  bool free_spc) const;
	///
	int Linuxdoc(Buffer const *, std::ostream & os) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	Inset * Clone() const;

	/// what appears in the minibuffer when opening
	char const * EditMessage() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	void InsetUnlock(BufferView *);
	///
	RESULT LocalDispatch(BufferView *, int, string const &);

protected:
	///
	//void UpdateLocal();

private:
	///
        bool opened;
	///
        string name;
	///
        MathMacroTemplate * tmacro;
};

#endif
