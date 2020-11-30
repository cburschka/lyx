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

///
enum class QuoteStyle : int {
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
enum class QuoteSide : int {
	///
	OpeningQuote,
	///
	ClosingQuote
};

///
enum class QuoteLevel : int {
	///
	SecondaryQuotes,
	///
	PrimaryQuotes
};


/** Quotes.
  Used for the various quotes. German, English, French, all either
  double or single **/
class InsetQuotesParams {
public:
	/// Returns the unicode character of a given quote
	char_type getQuoteChar(QuoteStyle const &, QuoteLevel const &,
			       QuoteSide const &, bool const rtl = false) const;
	/// Returns a map of quotation marks
	std::map<std::string, docstring> getTypes() const;
	///
	docstring getLaTeXQuote(char_type c, std::string const &,
				bool const rtl = false) const;
	///
	docstring getHTMLQuote(char_type c) const;
	///
	docstring getXMLQuote(char_type c) const;
	/// Returns a descriptive label of a style suitable for dialog and menu
	docstring const getGuiLabel(QuoteStyle const & qs,
				    bool langdef = false) const;
	/// Returns a descriptive label of a given char
	docstring const getShortGuiLabel(docstring const & str) const;
	///
	int stylescount() const;
	/// Returns the matching style shortcut char
	char getStyleChar(QuoteStyle const & style) const;
	/// Returns the quote style from the shortcut string
	QuoteStyle getQuoteStyle(std::string const & s,
		bool const allow_wildcards = false,
		QuoteStyle fallback = QuoteStyle::EnglishQuotes) const;
	/// Returns the quote sind from the shortcut string
	QuoteSide getQuoteSide(std::string const & s,
		bool const allow_wildcards = false,
		QuoteSide fallback = QuoteSide::OpeningQuote) const;
	/// Returns the quote level from the shortcut string
	QuoteLevel getQuoteLevel(std::string const & s,
		bool const allow_wildcards = false,
		QuoteLevel fallback = QuoteLevel::PrimaryQuotes) const;
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
	InsetQuotes(Buffer * buf, char_type c, QuoteLevel level,
		    std::string const & side = std::string(),
		    std::string const & style = std::string());
	///
	docstring layoutName() const override;
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
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
	void forOutliner(docstring &, size_t const maxlen, bool const) const override;

	/// Update the contextual information of this inset
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;

	///
	void validate(LaTeXFeatures &) const override;
	///
	std::string contextMenuName() const override;
	///
	InsetCode lyxCode() const override { return QUOTE_CODE; }
	/// should this inset be handled like a normal character
	bool isChar() const override { return true; }

	/// Returns the current quote type
	std::string getType() const;

private:
	///
	Inset * clone() const override { return new InsetQuotes(*this); }

	/// Decide whether we need left or right quotation marks
	void setSide(char_type c);
	///
	void parseString(std::string const &,
			 bool const allow_wildcards = false);
	///
	docstring displayString() const;
	///
	docstring getQuoteEntity(bool isHTML) const;
	///
	QuoteStyle getStyle(std::string const &);

	///
	QuoteStyle style_ = QuoteStyle::EnglishQuotes;
	///
	QuoteSide side_ = QuoteSide::OpeningQuote;
	///
	QuoteLevel level_ = QuoteLevel::PrimaryQuotes;
	///
	QuoteStyle global_style_ = QuoteStyle::EnglishQuotes;
	/// Current font encoding
	std::string fontenc_;
	/// Code of the contextual language
	std::string context_lang_;
	/// Is this in a pass-thru context?
	bool pass_thru_ = false;
	/// Do we use fontspec?
	bool fontspec_ = false;
	/// Do we have an internal font encoding?
	bool internal_fontenc_ = false;
	/// Are we writing RTL?
	bool rtl_ = false;
	///
	friend class InsetQuotesParams;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	//@}
};

} // namespace lyx

#endif
