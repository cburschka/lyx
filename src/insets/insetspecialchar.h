// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 Asger Alstrup
 *
 *======================================================*/

#ifndef _INSET_SPECIALCHAR_H
#define _INSET_SPECIALCHAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"

struct LaTeXFeatures;

///  Used to insert special chars
class InsetSpecialChar: public Inset {
public:

	/// The different kinds of special chars we support
	enum Kind {
		/// Optional hyphenation point (\-)
		HYPHENATION,
		/// ... (\ldots)
		LDOTS,
		/// End of sentence punctuation (\@)
		END_OF_SENTENCE,
		/// Menu separator
		MENU_SEPARATOR
	};

	///
	InsetSpecialChar();
	///
	InsetSpecialChar(Kind k);
	///
	~InsetSpecialChar();
	///
	int Ascent(LyXFont const &font) const;
	///
	int Descent(LyXFont const &font) const;
	///
	int Width(LyXFont const &font) const;
	///
	void Draw(LyXFont, LyXScreen &scr, int baseline, float &x);
	///
	void Write(FILE *file);
	/// Will not be used when lyxf3
	void Read(LyXLex &lex);
	/// 
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(string &file, signed char fragile);
	///
	int Linuxdoc(string &file);
	///
	int DocBook(string &file);
	///
	Inset* Clone();
	///  
	Inset::Code LyxCode() const
	{
		return Inset::NO_CODE;
	}
	/// We don't need \begin_inset and \end_inset
	bool DirectWrite() const 
	{
		return true;
	};
	///
	void Validate(LaTeXFeatures &) const;
private:
	/// And which kind is this?
	Kind kind;
};

#endif
