// -*- C++ -*-
/**
 * \file insetspecialchar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_SPECIALCHAR_H
#define INSET_SPECIALCHAR_H


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
	};

	///
	InsetSpecialChar() {}
	///
	explicit
	InsetSpecialChar(Kind k);
	///
	Kind kind() const;
	///
	void dimension(BufferView *, LyXFont const &, Dimension &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &) const;
	///
	void write(Buffer const *, std::ostream &) const;
	/// Will not be used when lyxf3
	void read(Buffer const *, LyXLex & lex);
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	virtual Inset * clone(Buffer const &) const;
	///
	//virtual Inset * clone(Buffer const &, bool same_id) const;
	///
	Inset::Code lyxCode() const { return Inset::SPECIALCHAR_CODE; }
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	///
	void validate(LaTeXFeatures &) const;

	// should this inset be handled like a normal charater
	bool isChar() const;
	/// is this equivalent to a letter?
	bool isLetter() const;
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
	// should we break lines after this inset?
	bool isLineSeparator() const;
private:
	/// And which kind is this?
	Kind kind_;
};

#endif
