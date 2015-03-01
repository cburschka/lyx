// -*- C++ -*-
/**
 * \file InsetSpecialChar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_SPECIALCHAR_H
#define INSET_SPECIALCHAR_H


#include "Inset.h"


namespace lyx {

class LaTeXFeatures;

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
		/// breakable slash
		SLASH,
		/// protected dash
		NOBREAKDASH,
		/// LyX logo
		PHRASE_LYX,
		/// TeX logo
		PHRASE_TEX,
		/// LaTeX2e logo
		PHRASE_LATEX2E,
		/// LaTeX logo
		PHRASE_LATEX
	};

	///
	InsetSpecialChar() : Inset(0) {}
	///
	explicit InsetSpecialChar(Kind k);
	///
	Kind kind() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	/// Will not be used when lyxf3
	void read(Lexer & lex);
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t) const;
	///
	InsetCode lyxCode() const { return SPECIALCHAR_CODE; }
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	///
	void validate(LaTeXFeatures &) const;

	/// should this inset be handled like a normal character?
	bool isChar() const { return true; }
	/// is this equivalent to a letter?
	bool isLetter() const;
	/// should we break lines after this inset?
	bool isLineSeparator() const;
private:
	Inset * clone() const { return new InsetSpecialChar(*this); }

	/// And which kind is this?
	Kind kind_;
};


} // namespace lyx

#endif
