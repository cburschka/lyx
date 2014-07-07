// -*- C++ -*-
/**
 * \file Length.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LENGTH_H
#define LENGTH_H

#include "support/strfwd.h"


namespace lyx {

// Solaris/x86 version 9 and earlier define these
#undef PC
#undef SP

/////////////////////////////////////////////////////////////////////
//
// Length
//
/////////////////////////////////////////////////////////////////////


/**
 * Length - Represents latex length measurement
 */
class Length {
public:
	/// length units
	enum UNIT {
		BP, ///< Big point (72bp = 1in), also PostScript point
		CC, ///< Cicero = 12dd = 4.531mm
		CM, ///< Centimeter = 10mm = 2.371pc
		DD, ///< Didot point = 1/72 of a French inch, = 0.376mm
		EM, ///< Width of capital "M" in current font.
		EX, ///< Height of a small "x" for the current font.
		IN, ///< Inch = 25.4mm = 72.27pt = 6.022pc
		MM, ///< Millimeter = 2.845pt
		MU, ///< Math unit (18mu = 1em) for positioning in math mode
		PC, ///< Pica = 12pt = 4.218mm
		PT, ///< Point = 1/72.27in = 0.351mm
		SP, ///< Scaled point (65536sp = 1pt) TeX's smallest unit.
		PTW, //< Percent of TextWidth
		PCW, //< Percent of ColumnWidth
		PPW, //< Percent of PageWidth
		PLW, //< Percent of LineWidth
		PTH, //< Percent of TextHeight          // Herbert 2002-05-16
		PPH, //< Percent of PaperHeight         // Herbert 2002-05-16
		UNIT_NONE ///< no unit
	};

	///
	Length();
	///
	Length(double v, Length::UNIT u);

	/// "data" must be a decimal number, followed by a unit
	explicit Length(std::string const & data);

	///
	double value() const;
	///
	Length::UNIT unit() const;
	///
	void value(double);
	///
	void unit(Length::UNIT unit);
	///
	bool zero() const;
	///
	bool empty() const;
	/// return string representation
	std::string const asString() const;
	/// return string representation
	docstring const asDocstring() const;
	/// return string representation for LaTeX
	std::string const asLatexString() const;
	/// return string representation for HTML
	std::string const asHTMLString() const;
	/// return the on-screen size of this length
	int inPixels(int text_width, int em_width = 0) const;
	/// return the value in Big Postscript points.
	int inBP() const;

	/// return the default unit (centimeter or inch)
	static UNIT defaultUnit();

	friend bool isValidLength(std::string const & data, Length * result);

private:
	///
	double val_;
	///
	Length::UNIT unit_;
};

///
bool operator==(Length const & l1, Length const & l2);
///
bool operator!=(Length const & l1, Length const & l2);
/** Test whether \p data represents a valid length.
 *
 * \returns whether \p data is a valid length
 * \param data Length in LyX format. Since the only difference between LyX
 * and LaTeX format is the representation of length variables as units (e.g.
 * \c text% vs. \c \\textwidth) you can actually use this function as well
 * for testing LaTeX lengths as long as they only contain real units like pt.
 * \param result Pointer to a Length variable. If \p result is not 0 and
 * \p data is valid, the length represented by it is stored into \p result.
 */
bool isValidLength(std::string const & data, Length * result = 0);
/// return the LyX name of the given unit number
char const * stringFromUnit(int unit);


/////////////////////////////////////////////////////////////////////
//
// GlueLength
//
/////////////////////////////////////////////////////////////////////

class GlueLength {
public:
	///
	GlueLength() {}
	///
	explicit GlueLength(Length const & len);
	///
	GlueLength(Length const & len,
		      Length const & plus,
		      Length const & minus);

	/** "data" must be a decimal number, followed by a unit, and
	  optional "glue" indicated by "+" and "-".  You may abbreviate
	  reasonably.  Examples:
	  1.2 cm  //  4mm +2pt  //  2cm -4mm +2mm  //  4+0.1-0.2cm
	  The traditional Latex format is also accepted, like
	  4cm plus 10pt minus 10pt */
	explicit GlueLength(std::string const & data);

	///
	Length const & len() const;
	///
	Length const & plus() const;
	///
	Length const & minus() const;


	/// conversion
	std::string const asString() const;
	///
	std::string const asLatexString() const;

	friend bool isValidGlueLength(std::string const & data,
				      GlueLength* result);

private:
	/// the normal vlaue
	Length len_;
	/// extra stretch
	Length plus_;
	/// extra shrink
	Length minus_;
};

///
bool operator==(GlueLength const & l1, GlueLength const & l2);
///
bool operator!=(GlueLength const & l1, GlueLength const & l2);
/** If "data" is valid, the length represented by it is
    stored into "result", if that is not 0. */
bool isValidGlueLength(std::string const & data, GlueLength * result = 0);

/// the number of units possible
extern int const num_units;

/**
 * array of unit names
 *
 * FIXME: I am not sure if "mu" should be possible to select (Lgb)
 */
extern char const * const unit_name[];
extern char const * const unit_name_gui[];

/// return the unit given a string representation such as "cm"
Length::UNIT unitFromString(std::string const & data);


} // namespace lyx

#endif // LENGTH_H
