// -*- C++ -*-
/**
 * \file lyxlength.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Matthias Ettrich
 */


#ifndef LYX_LENGTH_H
#define LYX_LENGTH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

/**
 * LyXLength - Represents latex length measurement
 */
class LyXLength {
public:
	/// length units
	enum UNIT {
		SP, ///< Scaled point (65536sp = 1pt) TeX's smallest unit.
		PT, ///< Point = 1/72.27in = 0.351mm
		BP, ///< Big point (72bp = 1in), also PostScript point
		DD, ///< Didot point = 1/72 of a French inch, = 0.376mm
		MM, ///< Millimeter = 2.845pt
		PC, ///< Pica = 12pt = 4.218mm
		CC, ///< Cicero = 12dd = 4.531mm
		CM, ///< Centimeter = 10mm = 2.371pc
		IN, ///< Inch = 25.4mm = 72.27pt = 6.022pc
		EX, ///< Height of a small "x" for the current font.
		EM, ///< Width of capital "M" in current font.
		MU, ///< Math unit (18mu = 1em) for positioning in math mode
		PW, ///< Percent of columnwidth
		PE, ///< Percent of textwidth
		PP, ///< Percent of pagewidth
		PL, ///< Percent of linewidth
		TH, ///< Percent of textheight		// Herbert 2002-05-16
		PH, ///< Percent of paperheight		// Herbert 2002-05-16
		UNIT_NONE ///< no unit
	};

	///
	LyXLength();
	///
	LyXLength(double v, LyXLength::UNIT u);

	/// "data" must be a decimal number, followed by a unit
	explicit LyXLength(string const & data);

	///
	double value() const;
	///
	LyXLength::UNIT unit() const;
	///
	void value(double);
	///
	void unit(LyXLength::UNIT unit);
	///
	bool zero() const;
	/// return string representation
	string const asString() const;
	/// return string representation for LaTeX
	string const asLatexString() const;
	/// return the on-screen size of this length
	int inPixels(int default_width, int default_height) const;
	/// return the on-screen size of this length of an image
	int inBP() const;

	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidLength(string const & data, LyXLength * result = 0);

private:
	///
	double          val_;
	///
	LyXLength::UNIT unit_;
};

///
bool operator==(LyXLength const & l1, LyXLength const & l2);
///
bool operator!=(LyXLength const & l1, LyXLength const & l2);
///
bool isValidLength(string const & data, LyXLength * result);
/// return the name of the given unit number
char const * stringFromUnit(int unit);

#endif // LYXLENGTH_H
