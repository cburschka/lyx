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

#include <iosfwd>

#include "insets/lyxinset.h"

class MathParInset;
class MathedCursor;
class Buffer;

///
class InsetFormula: public UpdatableInset {
public:
	///
	explicit
	InsetFormula(bool display = false);
	///
	explicit
	InsetFormula(MathParInset *);
	///
	~InsetFormula();
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *,LyXFont const &, int, float &, bool) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures &) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MATH_CODE; }
	///
	LyXFont const ConvertFont(LyXFont const & f) const {
		// We have already discussed what was here
		LyXFont font(f);
	        font.setLatex(LyXFont::OFF);
	        return font;
	}

	/// what appears in the minibuffer when opening
	string const EditMessage() const;
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
	void GetCursorPos(BufferView *, int &, int &) const;
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
	void InsertSymbol(BufferView *, string const &);
	///
	bool SetNumber(bool);
	///
	std::vector<string> const getLabelList() const;
   
protected:
	///
	void UpdateLocal(BufferView * bv);
	///
    	MathParInset * par;
	///
	static MathedCursor * mathcursor;
    
private:
	///
	bool disp_flag;
	///
	string label;
};


// If a mathinset exist at cursor pos, just lock it.
// Otherwise create a new one, and lock it.
bool OpenMathInset(Buffer *);

#endif
