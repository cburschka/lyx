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
	enum QuoteStyle {
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
	enum QuoteLevel {
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
	explicit InsetQuotes(Buffer * buf, std::string const & str = "eld");
	/// Direct access to inner/outer quotation marks
	InsetQuotes(Buffer * buf, char_type c, QuoteLevel level,
		    std::string const & side = std::string(),
		    std::string const & style = std::string());
	///
	docstring layoutName() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
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
	void forOutliner(docstring &, size_t const maxlen, bool const) const;

	/// Update the contextual information of this inset
	void updateBuffer(ParIterator const &, UpdateType);

	///
	void validate(LaTeXFeatures &) const;
	///
	std::string contextMenuName() const;
	///
	InsetCode lyxCode() const { return QUOTE_CODE; }
	/// should this inset be handled like a normal character
	bool isChar() const { return true; }
	
	/// Returns the current quote type
	std::string getType() const;
	/// Returns a map of quotation marks
	std::map<std::string, docstring> getTypes() const;

private:
	///
	Inset * clone() const { return new InsetQuotes(*this); }

	/// Decide whether we need left or right quotation marks
	void setSide(char_type c);
	///
	void parseString(std::string const &,
			 bool const allow_wildcards = false);
	///
	docstring displayString() const;
	///
	docstring getQuoteEntity() const;
	///
	QuoteStyle getStyle(std::string const &);

	///
	QuoteStyle style_;
	///
	QuoteSide side_;
	///
	QuoteLevel level_;
	///
	std::string fontenc_;
	/// Code of the contextual language
	std::string context_lang_;
	/// Is this in a pass-thru context?
	bool pass_thru_;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	//@}
};

} // namespace lyx

#endif
