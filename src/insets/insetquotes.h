// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team
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
	InsetQuotes(string const & str = "eld");
	/// Create the right quote inset after character c
	InsetQuotes(char c, BufferParams const & params);

	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &, int baseline, float & x) const;
	///
	LyXFont ConvertFont(LyXFont font);
	///
	void Write(ostream &) const;
	///
	void Read(LyXLex & lex);
	///
	int Latex(ostream &, signed char fragile) const;
	///
	int Latex(string & file, signed char fragile) const;
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
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
