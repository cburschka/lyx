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

#include "inset.h"
#include "LString.h"

struct LaTeXFeatures;

///  Used to insert special chars
class InsetSpecialChar : public Inset {
public:

	/// The different kinds of special chars we support
	enum Kind {
		/// Optional hyphenation point (\-)
		HYPHENATION,
		/// Ligature break point (\textcompwordmark)
		LIGATURE_BREAK,
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
	Kind kind() const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void write(Buffer const *, std::ostream &) const;
	/// Will not be used when lyxf3
	void read(Buffer const *, LyXLex & lex);
	/// 
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docBook(Buffer const *, std::ostream &) const;
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;
	///  
	Inset::Code lyxCode() const
	{
		return Inset::SPECIALCHAR_CODE;
	}
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const 
	{
		return true;
	};
	///
	void validate(LaTeXFeatures &) const;
private:
	/// And which kind is this?
	Kind kind_;
};

#endif
