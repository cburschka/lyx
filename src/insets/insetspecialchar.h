// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 Asger Alstrup
 *
 * ====================================================== */

#ifndef INSET_SPECIALCHAR_H
#define INSET_SPECIALCHAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"

struct LaTeXFeatures;

///  Used to insert special chars
class InsetSpecialChar : public Inset {
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
		MENU_SEPARATOR,
#if 0
		/// Newline
		NEWLINE,
#endif
		/// Protected Separator
		PROTECTED_SEPARATOR
	};

	///
	InsetSpecialChar() {}
	///
	InsetSpecialChar(Kind k);
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
	/// Will not be used when lyxf3
	void Read(LyXLex & lex);
	/// 
	int Latex(ostream &, signed char fragile) const;
	///
	int Latex(string & file, signed char fragile) const;
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
	///
	Inset * Clone() const;
	///  
	Inset::Code LyxCode() const
	{
		return Inset::SPECIALCHAR_CODE;
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
