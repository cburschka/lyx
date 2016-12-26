// -*- C++ -*-
/**
 * \file InsetQuotes.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *  \author Jürgen Spitzmüller
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
class InsetQuotesParams {
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
		SwissQuotes,
		///
		DanishQuotes,
		///
		PlainQuotes,
		///
		BritishQuotes,
		///
		SwedishGQuotes,
		///
		FrenchQuotes,
		///
		FrenchINQuotes,
		///
		RussianQuotes,
		///
		CJKQuotes,
		///
		CJKAngleQuotes,
		///
		DynamicQuotes
	};
	///
	enum QuoteSide {
		///
		OpeningQuote,
		///
		ClosingQuote
	};
	///
	enum QuoteLevel {
		///
		SecondaryQuotes,
		///
		PrimaryQuotes
	};
	/// Returns the unicode character of a given quote
	char_type getQuoteChar(QuoteStyle const &, QuoteLevel const &,
			       QuoteSide const &) const;
	/// Returns a map of quotation marks
	std::map<std::string, docstring> getTypes() const;
	///
	docstring getLaTeXQuote(char_type c, std::string const &) const;
	///
	docstring getHTMLQuote(char_type c) const;
	/// Returns a label suitable for dialog and menu
	docstring const getGuiLabel(QuoteStyle const & qs);
	///
	int stylescount() const;
};

///
extern InsetQuotesParams quoteparams;

/** Quotes.
  Used for the various quotes. German, English, French, all either
  double or single **/
class InsetQuotes : public Inset
{
public:
	/** The constructor works like this:
	  \begin{itemize}
	    \item fls <- french single quote left
	    \item grd <- german double quote right
	    \item etc.
	  \end{itemize}
	  */
	explicit InsetQuotes(Buffer * buf, std::string const & str = "eld");
	/// Direct access to inner/outer quotation marks
	InsetQuotes(Buffer * buf, char_type c, InsetQuotesParams::QuoteLevel level,
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
	InsetQuotesParams::QuoteStyle getStyle(std::string const &);

	///
	InsetQuotesParams::QuoteStyle style_;
	///
	InsetQuotesParams::QuoteSide side_;
	///
	InsetQuotesParams::QuoteLevel level_;
	///
	InsetQuotesParams::QuoteStyle global_style_;
	///
	std::string fontenc_;
	/// Code of the contextual language
	std::string context_lang_;
	/// Is this in a pass-thru context?
	bool pass_thru_;
	/// Do we use fontspec?
	bool fontspec_;
	///
	friend class InsetQuotesParams;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	//@}
};

} // namespace lyx

#endif
