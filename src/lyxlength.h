// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_LENGTH_H
#define LYX_LENGTH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

//
///  LyXLength Class
//
class LyXLength {
public:
	/// length units
	enum UNIT {
		/// Scaled point (65536sp = 1pt) TeX's smallest unit.
		SP,
		/// Point = 1/72.27in = 0.351mm
		PT,
		/// Big point (72bp = 1in), also PostScript point
		BP,
		/// Didot point = 1/72 of a French inch, = 0.376mm
		DD,
		/// Millimeter = 2.845pt
		MM,
		/// Pica = 12pt = 4.218mm
		PC,
		/// Cicero = 12dd = 4.531mm
		CC,
		/// Centimeter = 10mm = 2.371pc
		CM,
		/// Inch = 25.4mm = 72.27pt = 6.022pc
		IN,
		/// Height of a small "x" for the current font.
		EX,
		/// Width of capital "M" in current font.
		EM,
		/// Math unit (18mu = 1em) for positioning in math mode
		MU,
		/// Percent of columnwidth both "%" or "%c"
		PW,
		///
		PE,
		/// Percent of pagewidth
		PP,
		/// Percent of linewidth
		PL,
		/// no unit
		UNIT_NONE
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
	/// conversion
	string const asString() const;
	///
	string const asLatexString() const;
	///
	int inPixels(int default_width, int default_height) const;

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
///
char const * stringFromUnit(int unit);

#endif
