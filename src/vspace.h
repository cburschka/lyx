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

#ifndef VSPACE_H
#define VSPACE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class BufferParams;
class BufferView;


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

	/// conversion
	virtual string const asString() const;
	///
	virtual string const asLatexString() const;

	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidLength(string const & data, LyXLength * result = 0);

protected:
	///
	double          val_;
	///
	LyXLength::UNIT unit_;
};

///
bool operator==(LyXLength const & l1, LyXLength const & l2);
///
LyXLength::UNIT unitFromString (string const & data);
///
bool isValidLength(string const & data, LyXLength * result);
///
const char * stringFromUnit(int unit);



//
/// LyXGlueLength class
//
class LyXGlueLength : public LyXLength {
public:
	///
	LyXGlueLength(double v,
		      LyXLength::UNIT u,
		      double pv = 0.0,
		      LyXLength::UNIT pu = LyXLength::UNIT_NONE,
		      double mv = 0.0,
		      LyXLength::UNIT mu = LyXLength::UNIT_NONE);

	/** "data" must be a decimal number, followed by a unit, and
	  optional "glue" indicated by "+" and "-".  You may abbreviate
	  reasonably.  Examples:
	  1.2 cm  //  4mm +2pt  //  2cm -4mm +2mm  //  4+0.1-0.2cm
	  The traditional Latex format is also accepted, like
	  4cm plus 10pt minus 10pt */
	explicit LyXGlueLength(string const & data);
	
	///
	double plusValue() const;
	///
	LyXLength::UNIT plusUnit() const;
	///
	double minusValue() const;
	///
	LyXLength::UNIT minusUnit() const;

	/// conversion
	virtual string const asString() const;
	///
	virtual string const asLatexString() const;


	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidGlueLength(string const & data,
				      LyXGlueLength* result = 0);

protected:
	///
	double plus_val_;
	///
	double minus_val_;
	///
	LyXLength::UNIT plus_unit_;
	///
	LyXLength::UNIT minus_unit_;
};

///
bool operator==(LyXGlueLength const & l1, LyXGlueLength const & l2);
///
bool operator!=(LyXGlueLength const & l1, LyXGlueLength const & l2);
///
bool isValidGlueLength(string const & data, LyXGlueLength * result);


//
///  VSpace class
//
class VSpace {
public:
	/// The different kinds of spaces.
	enum vspace_kind {
		///
		NONE,
		///
		DEFSKIP,
		///
		SMALLSKIP,
		///
		MEDSKIP,
		///
		BIGSKIP,
		///
		VFILL,
		///
		LENGTH
	};
	/// Constructor
	VSpace();
	/// Constructor
	explicit VSpace(vspace_kind k);
	/// Constructor
	explicit VSpace(LyXGlueLength l);
	/// Constructor
	explicit VSpace(double v, LyXLength::UNIT u);

	/// Constructor for reading from a .lyx file
	explicit VSpace(string const & data);
	
	/// access functions
	vspace_kind kind() const;
	///
	LyXGlueLength  length() const;

	// a flag that switches between \vspace and \vspace*
	bool keep() const;
	///
	void setKeep(bool val);
	///
	bool operator==(VSpace const &) const;

	// conversion
	/// how it goes into the LyX file
	string const asLyXCommand() const; 
	///
	string const asLatexCommand(BufferParams const & params) const;
	///
	int inPixels(BufferView * bv) const;
	///
	int inPixels(int default_height, int default_skip, int default_width=0) const;
private:
	/// This VSpace kind
	vspace_kind kind_;
	///
	LyXGlueLength len_;
	///
	bool keep_;
};

#endif
