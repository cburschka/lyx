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

#ifndef INSET_FORMULA_H 
#define INSET_FORMULA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insets/lyxinset.h"
#include "support/LOstream.h"

class MathParInset;
class MathedCursor;

///
class InsetFormula: public UpdatableInset {
public:
	///
	InsetFormula(bool display = false);
	///
	InsetFormula(MathParInset *);
	///
	~InsetFormula();
	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &, int baseline, float & x) const;
	///
	void Write(ostream &) const;
	///
	void Read(LyXLex & lex);
	///
	int Latex(ostream &, signed char fragile, bool free_spc) const;
#ifndef USE_OSTREAM_ONLY
	///
	int Latex(string & file, signed char fragile, bool free_spc) const;
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
#else
	///
	int Linuxdoc(ostream &) const;
	///
	int DocBook(ostream &) const;
#endif
	///
	void Validate(LaTeXFeatures &) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MATH_CODE; }
	///
	LyXFont ConvertFont(LyXFont font) {
		// We have already discussed what was here
	        font.setLatex(LyXFont::OFF);
	        return font;
	}

	/// what appears in the minibuffer when opening
	const char * EditMessage() const {return _("Math editor mode");}
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	bool display() const { return (disp_flag) ? true: false; }
	///
	void display(bool);
	///
	void ToggleInsetCursor(BufferView *);
	///
	void ShowInsetCursor(BufferView *);
	///
	void HideInsetCursor(BufferView *);
	///
	void GetCursorPos(int &, int &) const;
	///
	void ToggleInsetSelection(BufferView * bv);
	///
	void InsetButtonPress(BufferView *, int x, int y, int button);
	///
	void InsetButtonRelease(BufferView *, int x, int y, int button);
	///
	void InsetKeyPress(XKeyEvent * ev);
	///
	void InsetMotionNotify(BufferView *, int x, int y, int state);
	///
	void InsetUnlock(BufferView *);
   
	///  To allow transparent use of math editing functions
	virtual RESULT LocalDispatch(BufferView *, int, string const &);
    
	///
	void InsertSymbol(BufferView *, char const *);
	///
	bool SetNumber(bool);
	///
	int GetNumberOfLabels() const;
	///
	string getLabel(int) const;
   
protected:
	void UpdateLocal(BufferView * bv);
    	MathParInset * par;
	static MathedCursor * mathcursor;
    
private:
	bool disp_flag;
	string label;
};


// If a mathinset exist at cursor pos, just lock it.
// Otherwise create a new one, and lock it.
bool OpenMathInset(Buffer *);

#endif
