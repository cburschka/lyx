// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team
 *
 * ====================================================== */

#ifndef INSET_QUOTES_H
#define INSET_QUOTES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"

class BufferParams;

struct LaTeXFeatures;


/** Quotes.

  Used for the various quotes. German, English, French, all either
  double or single **/ 
class InsetQuotes: public Inset {
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
	InsetQuotes(string const & str = "eld");
	/// Create the right quote inset after character c
	InsetQuotes(char c, BufferParams const & params);

	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	LyXFont ConvertFont(LyXFont font);
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures &) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const;
private:
	///
	quote_language language;
	///
	quote_side side;
	///
	quote_times times;

	/** The parameters of the constructor are the language, the
	    side and the multiplicity of the quote.
	 */
	InsetQuotes(quote_language l, quote_side s, quote_times t);
	///
	void ParseString(string const &);
	///
	string DispString() const;
};
#endif
