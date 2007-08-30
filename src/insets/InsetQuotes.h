// -*- C++ -*-
/**
 * \file InsetQuotes.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_QUOTES_H
#define INSET_QUOTES_H


#include "Inset.h"

#include "support/types.h"


namespace lyx {

class BufferParams;
class Language;
class LaTeXFeatures;


/** Quotes.
  Used for the various quotes. German, English, French, all either
  double or single **/
class InsetQuotes : public Inset {
public:
	///
	enum quote_language {
		///
		EnglishQ,
		///
		SwedishQ,
		///
		GermanQ,
		///
		PolishQ,
		///
		FrenchQ,
		///
		DanishQ
	};
	///
	enum quote_side {
		///
		LeftQ,
		///
		RightQ
	};
	///
	enum quote_times {
		///
		SingleQ,
		///
		DoubleQ
	};

	/** The constructor works like this:
	  \begin{itemize}
	    \item fls <- french single quote left
	    \item grd <- german double quote right
	    \item etc.
	  \end{itemize}
	  */
	explicit
	InsetQuotes(std::string const & str = "eld");
	/// Create the right quote inset after character c
	InsetQuotes(char_type c, BufferParams const & params);
	/// Direct access to inner/outer quotation marks
	InsetQuotes(char_type c, quote_language l, quote_times t);
	///
	docstring name() const { return from_ascii("Quotes"); }
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
#if 0
	///
	Font const convertFont(Font const & font) const;
#endif
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, Lexer & lex);
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &, OutputParams const &) const;

	/// the string that is passed to the TOC
	virtual void textString(Buffer const &, odocstream &) const;

	///
	void validate(LaTeXFeatures &) const;
	///
	Inset::Code lyxCode() const;
	// should this inset be handled like a normal character
	bool isChar() const { return true; }

private:
	virtual Inset * clone() const;

	///
	quote_language language_;
	///
	quote_side side_;
	///
	quote_times times_;

	/** The parameters of the constructor are the language, the
	    side and the multiplicity of the quote.
	 */
	InsetQuotes(quote_language l, quote_side s, quote_times t);
	/// Decide whether we need left or right quotation marks
	void getPosition(char_type c);
	///
	void parseString(std::string const &);
	///
	lyx::docstring const dispString(Language const *) const;
};

} // namespace lyx

#endif
