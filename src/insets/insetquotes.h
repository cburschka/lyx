// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team
 *
 * ======================================================*/

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
	~InsetQuotes() {}; //nothing to do

	///
	int Ascent(LyXFont const & font) const;
	///
	int Descent(LyXFont const & font) const;
	///
	int Width(LyXFont const & font) const;
	///
	void Draw(LyXFont font, LyXScreen & scr, int baseline, float & x);
	///
	LyXFont ConvertFont(LyXFont font);
	///
	void Write(FILE * file);
	///
	void Read(LyXLex & lex);
	///
	int Latex(FILE * file, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
	///
	void Validate(LaTeXFeatures &) const;
	///
	Inset* Clone();
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
	void ParseString(string str);
	///
	string DispString() const;
};

#endif
