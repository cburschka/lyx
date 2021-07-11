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
		/// Optional line wrap point without hyphen (ZWSP)
		ALLOWBREAK,
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
	InsetSpecialChar() : Inset(0), kind_(HYPHENATION) {}
	///
	explicit InsetSpecialChar(Kind k);
	///
	Kind kind() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	/// some special chars allow line breaking after them
	int rowFlags() const override;
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(std::ostream &) const override;
	/// Will not be used when lyxf3
	void read(Lexer & lex) override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void toString(odocstream &) const override;
	///
	bool isInToc() const override { return true; }
	///
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	InsetCode lyxCode() const override { return SPECIALCHAR_CODE; }
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const override { return true; }
	///
	void validate(LaTeXFeatures &) const override;

	/// should this inset be handled like a normal character?
	bool isChar() const override;
	/// is this equivalent to a letter?
	bool isLetter() const override;
	/// should we break lines after this inset?
	bool isLineSeparator() const override;
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return isChar(); }
private:
	Inset * clone() const override { return new InsetSpecialChar(*this); }

	/// And which kind is this?
	Kind kind_;
};


} // namespace lyx

#endif
