// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 Asger Alstrup
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
		/// Protected Separator
		PROTECTED_SEPARATOR
	};

	///
	InsetSpecialChar() {}
	///
	explicit
	InsetSpecialChar(Kind k);
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	/// Will not be used when lyxf3
	void Read(Buffer const *, LyXLex & lex);
	/// 
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
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
