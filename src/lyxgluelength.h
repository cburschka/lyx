// -*- C++ -*-
/**
 * \file lyxgluelength.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_GLUE_LENGTH_H
#define LYX_GLUE_LENGTH_H

#include "lyxlength.h"

#include "support/std_string.h"


class LyXGlueLength {
public:
	///
	LyXGlueLength() {}
	///
	explicit LyXGlueLength(LyXLength const & len);
	///
	LyXGlueLength(LyXLength const & len,
		      LyXLength const & plus,
		      LyXLength const & minus);

	/** "data" must be a decimal number, followed by a unit, and
	  optional "glue" indicated by "+" and "-".  You may abbreviate
	  reasonably.  Examples:
	  1.2 cm  //  4mm +2pt  //  2cm -4mm +2mm  //  4+0.1-0.2cm
	  The traditional Latex format is also accepted, like
	  4cm plus 10pt minus 10pt */
	explicit LyXGlueLength(string const & data);

	///
	LyXLength const & len() const;
	///
	LyXLength const & plus() const;
	///
	LyXLength const & minus() const;


	/// conversion
	string const asString() const;
	///
	string const asLatexString() const;

	friend bool isValidGlueLength(string const & data,
				      LyXGlueLength* result);

private:
	/// the normal vlaue
	LyXLength len_;
	/// extra stretch
	LyXLength plus_;
	/// extra shrink
	LyXLength minus_;
};

///
bool operator==(LyXGlueLength const & l1, LyXGlueLength const & l2);
///
bool operator!=(LyXGlueLength const & l1, LyXGlueLength const & l2);
/** If "data" is valid, the length represented by it is
    stored into "result", if that is not 0. */
bool isValidGlueLength(string const & data, LyXGlueLength * result = 0);

#endif // LYXGLUELENGTH_H
