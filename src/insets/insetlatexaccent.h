// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *
 * ====================================================== */

#ifndef INSET_LATEX_ACCENT_H
#define INSET_LATEX_ACCENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"
#include "lyxlex.h"

/** Insertion of accents
  
  Proper handling of accented characters.
  This is class is supposed to handle all LaTeX accents, it
  is also possible that the class will change a bit so that
  it also can handle other special characters (e.g. Hstroke)
  Initiated by Ivan Schreter, later modified by Lgb.
  */
class InsetLatexAccent : public Inset {
public:
	///
	InsetLatexAccent(); 
	///
	explicit
	InsetLatexAccent(string const & string);
	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &, int baseline, float & x) const;
	///
	int Lbearing(LyXFont const & font) const;
	///
	int Rbearing(LyXFont const & font) const;
	///
	bool DisplayISO8859_9(Painter &, LyXFont const & font,
			      int baseline, float & x) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	bool Deletable() const;
	///
	bool DirectWrite() const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode()const;
	///
	inline bool CanDisplay();
	/// all the accent types
	enum ACCENT_TYPES{
		///
		ACUTE, // 0
		///
		GRAVE,
		///
		MACRON,
		///
		TILDE,
		///
		UNDERBAR,
		///
		CEDILLA, // 5
		///
		UNDERDOT,
		///
		CIRCLE,
		///
		TIE,
		///
		BREVE,
		///
		CARON, // 10
		///
		SPECIAL_CARON,
		///
		HUNGARIAN_UMLAUT,
		///
		UMLAUT,
		///
		DOT,
		///
		CIRCUMFLEX, // 15
		///
		OGONEK,
		///
		DOT_LESS_I,
		///
 		DOT_LESS_J, // 18
		///
 		lSLASH,
		///
 		LSLASH
	};
private:
	friend std::ostream & operator<<(std::ostream &, ACCENT_TYPES);
	/// Check if we know the modifier and can display it ok on screen.
	void checkContents();
	///
	string contents;
	/// can display as proper char
	bool  candisp;
	/// modifier type
	ACCENT_TYPES  modtype;
	
	/// remove dot from 'i' and 'j' or transform l, L into lslash, LSLaSH
	bool  remdot;
	/// add something to ascent - accent at the top
	bool  plusasc;
	/// add something to descent - underlined char
	bool  plusdesc;
	/// international char
	mutable char  ic;    
};

bool InsetLatexAccent::CanDisplay()
{
	return candisp;
}

#endif
