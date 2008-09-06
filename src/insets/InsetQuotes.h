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

#include "support/docstring.h"


namespace lyx {

/** Quotes.
  Used for the various quotes. German, English, French, all either
  double or single **/
class InsetQuotes : public Inset
{
public:
	///
	enum QuoteLanguage {
		///
		EnglishQuotes,
		///
		SwedishQuotes,
		///
		GermanQuotes,
		///
		PolishQuotes,
		///
		FrenchQuotes,
		///
		DanishQuotes
	};
	///
	enum QuoteSide {
		///
		LeftQuote,
		///
		RightQuote
	};
	///
	enum QuoteTimes {
		///
		SingleQuotes,
		///
		DoubleQuotes
	};

	/** The constructor works like this:
	  \begin{itemize}
	    \item fls <- french single quote left
	    \item grd <- german double quote right
	    \item etc.
	  \end{itemize}
	  */
	explicit InsetQuotes(Buffer const & buf, std::string const & str = "eld");
	/// Create the right quote inset after character c
	InsetQuotes(Buffer const & buffer, char_type c);
	/// Direct access to inner/outer quotation marks
	InsetQuotes(Buffer const & buf, char_type c, QuoteLanguage l, QuoteTimes t);
	///
	docstring name() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;

	/// the string that is passed to the TOC
	void textString(odocstream &) const;

	///
	void validate(LaTeXFeatures &) const;
	///
	InsetCode lyxCode() const { return QUOTE_CODE; }
	/// should this inset be handled like a normal character
	bool isChar() const { return true; }

private:
	///
	Inset * clone() const { return new InsetQuotes(*this); }

	/// Decide whether we need left or right quotation marks
	void setSide(char_type c);
	///
	void parseString(std::string const &);
	///
	docstring displayString() const;

	///
	QuoteLanguage language_;
	///
	QuoteSide side_;
	///
	QuoteTimes times_;
};

} // namespace lyx

#endif
