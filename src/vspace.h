// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 1996 Matthias Ettrich
 *           and the LyX Team.
 *
 *======================================================*/

#ifndef VSPACE_H
#define VSPACE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdio>

///  LyXLength Class
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
		/// no unit
		UNIT_NONE
	};

	//@Man: constructors
	//@{
	///
	LyXLength() : val(0), uni(LyXLength::PT) {}
	LyXLength(float v, LyXLength::UNIT u) : val(v), uni(u) {}

	/** "data" must be a decimal number, followed by a unit. */
        LyXLength(string const & data);
	//@}
	
	//@Man: selectors
	//@{
	///
	float value() const         { return val; };
	///
	LyXLength::UNIT unit() const { return uni; };
	//@}

	///
        bool operator==(LyXLength const &) const;

	/// conversion
	virtual string asString() const;
	///
	virtual string asLatexString() const { return this->asString(); };


	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidLength(string const & data, 
				  LyXLength * result=0);

protected:
	///
	float           val;
	///
	LyXLength::UNIT uni;
};

extern LyXLength::UNIT unitFromString (string const & data);
extern bool isValidLength(string const & data, LyXLength * result);

/// LyXGlueLength class
class LyXGlueLength : public LyXLength {
public:
	//@Man: constructors
	//@{
	///
	LyXGlueLength(float v,      LyXLength::UNIT u, 
		      float pv=0.0, LyXLength::UNIT pu=LyXLength::UNIT_NONE, 
		      float mv=0.0, LyXLength::UNIT mu=LyXLength::UNIT_NONE) 
		: LyXLength (v, u), 
		  plus_val(pv), minus_val(mv), 
		  plus_uni(pu), minus_uni(mu) {}

	/** "data" must be a decimal number, followed by a unit, and 
	  optional "glue" indicated by "+" and "-".  You may abbreviate
	  reasonably.  Examples:
	  1.2 cm  //  4mm +2pt  //  2cm -4mm +2mm  //  4+0.1-0.2cm
	  The traditional Latex format is also accepted, like  
	  4cm plus 10pt minus 10pt */
        LyXGlueLength(string const & data);
	//@}
	
	//@Man: selectors
	//@{
	///
	float plusValue() const         { return plus_val; };
	///
	LyXLength::UNIT plusUnit() const { return plus_uni; };
	///
	float minusValue() const         { return minus_val; };
	///
	LyXLength::UNIT minusUnit() const { return minus_uni; };
	//@}

	///
        bool operator==(LyXGlueLength const &) const;

	/// conversion
	virtual string asString() const;
	///
	virtual string asLatexString() const;


	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidGlueLength(string const & data, 
				      LyXGlueLength* result=0);

protected:
	///
	float           plus_val, minus_val;
	///
	LyXLength::UNIT plus_uni, minus_uni;
};

extern bool isValidGlueLength(string const & data, LyXGlueLength * result);

///  VSpace class
class VSpace {
public:
	///
	enum vspace_kind { NONE, DEFSKIP, 
			   SMALLSKIP, MEDSKIP, BIGSKIP, 
			   VFILL, LENGTH };
	// constructors
	VSpace() : 
		kin (NONE), 
		len(0.0, LyXLength::PT),
                kp (false) {}

	VSpace(vspace_kind k) : 
		kin (k), 
		len (0.0, LyXLength::PT),
	        kp (false) {}

	VSpace(LyXGlueLength l) :
		kin (LENGTH),
		len (l),
	        kp (false) {}

	VSpace(float v, LyXLength::UNIT u) : 
		kin (LENGTH), 
		len (v, u),
	        kp (false) {}

	/// this constructor is for reading from a .lyx file
	VSpace(string const & data);
	
	// access functions
	vspace_kind kind() const  { return  kin; }
	///
	LyXLength   length() const { return len; }

	// a flag that switches between \vspace and \vspace*
        bool keep() const      { return kp; }
	///
	void setKeep(bool val) { kp = val; } 
	///
        bool operator==(VSpace const &) const;

	// conversion
	///
	string asLyXCommand() const;  // how it goes into the LyX file
	///
	string asLatexCommand() const;
	///
	int inPixels() const;
private:
	///
	vspace_kind  kin;
	///
	LyXGlueLength    len;
	///
	bool kp;
};

#endif


