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

#ifndef INSET_FORMULA_H 
#define INSET_FORMULA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insets/lyxinset.h"


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
	int Ascent(LyXFont const & font) const;
	///
	int Descent(LyXFont const & font) const;
	///
	int Width(LyXFont const & font) const;
	///
	void Draw(LyXFont font, LyXScreen & scr, int baseline, float & x);
	///
	void Write(FILE * file);
	///
	void Read(LyXLex & lex);
	///
	int Latex(FILE * file, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
	///
	void Validate(LaTeXFeatures &) const;
	///
	InsetFormula * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MATH_CODE; }
	///
	LyXFont ConvertFont(LyXFont font) {
		// We have already discussed what was here
	        font.setLatex(LyXFont::OFF);
	        return font;
	}

	/// what appears in the minibuffer when opening
	char const * EditMessage() {return _("Math editor mode");}
	///
	void Edit(int x, int y);
	///
	bool display() const { return (disp_flag) ? true: false; }
	///
	void display(bool);
	///
	void ToggleInsetCursor();
	///
	void ShowInsetCursor();
	///
	void HideInsetCursor();
	///
	void GetCursorPos(int &, int &);
	///
	void ToggleInsetSelection();
	///
	void InsetButtonPress(int x, int y, int button);
	///
	void InsetButtonRelease(int x, int y, int button);
	///
	void InsetKeyPress(XKeyEvent * ev);
	///
	void InsetMotionNotify(int x, int y, int state);
	///
	void InsetUnlock();
   
	///  To allow transparent use of math editing functions
	virtual bool LocalDispatch(int, char const *);
    
	///
	void InsertSymbol(char const *);
	///
	bool SetNumber(bool);
	///
	int GetNumberOfLabels() const;
	///
	string getLabel(int) const;
   
protected:
	void UpdateLocal();
    	MathParInset * par;
	static MathedCursor* mathcursor;
    
private:
	bool disp_flag;
	string label;
};


// If a mathinset exist at cursor pos, just lock it.
// Otherwise create a new one, and lock it.
bool OpenMathInset(Buffer *);

#endif
