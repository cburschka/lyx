// -*- C++ -*-
/**
 * \file insetlatexaccent.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LATEX_ACCENT_H
#define INSET_LATEX_ACCENT_H

#include "inset.h"

class Dimension;


/** Insertion of accents

  Proper handling of accented characters.
  This is class is supposed to handle all LaTeX accents, it
  is also possible that the class will change a bit so that
  it also can handle other special characters (e.g. Hstroke)
  Initiated by Ivan Schreter, later modified by Lgb.
  */
class InsetLatexAccent : public InsetOld {
public:
	///
	InsetLatexAccent();
	///
	explicit InsetLatexAccent(std::string const & str);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int lbearing(LyXFont const & font) const;
	///
	int rbearing(LyXFont const & font) const;
	///
	bool displayISO8859_9(PainterInfo & pi, int x, int y) const;
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex & lex);
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
	///
	bool directWrite() const;
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	InsetOld::Code lyxCode()const;
	///
	inline bool canDisplay();
	// should this inset be handled like a normal charater
	bool isChar() const { return true; }

	// is this equivalent to a letter?
	virtual bool isLetter() const { return candisp; }

	/// all the accent types
	enum ACCENT_TYPES{
		///
		ACUTE, // 0
		///
		GRAVE,
		///
		MACRON,
		///
		TILDE,
		///
		UNDERBAR,
		///
		CEDILLA, // 5
		///
		UNDERDOT,
		///
		CIRCLE,
		///
		TIE,
		///
		BREVE,
		///
		CARON, // 10
		///
		SPECIAL_CARON,
		///
		HUNGARIAN_UMLAUT,
		///
		UMLAUT,
		///
		DOT,
		///
		CIRCUMFLEX, // 15
		///
		OGONEK,
		///
		DOT_LESS_I,
		///
		DOT_LESS_J, // 18
		///
		lSLASH,
		///
		LSLASH
	};
private:
	friend std::ostream & operator<<(std::ostream &, ACCENT_TYPES);
	/// Check if we know the modifier and can display it ok on screen.
	void checkContents();
	///
	void drawAccent(PainterInfo const & pi, int x, int y, char accent) const;
	///
	std::string contents;
	/// can display as proper char
	bool  candisp;
	/// modifier type
	ACCENT_TYPES  modtype;

	/// remove dot from 'i' and 'j' or transform l, L into lslash, LSLaSH
	bool  remdot;
	/// add something to ascent - accent at the top
	bool  plusasc;
	/// add something to descent - underlined char
	bool  plusdesc;
	/// international char
	mutable char  ic;
};


bool InsetLatexAccent::canDisplay()
{
	return candisp;
}

#endif
