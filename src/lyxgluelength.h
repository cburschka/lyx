// -*- C++ -*-
/**
 * \file lyxgluelength.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Matthias Ettrich
 */


#ifndef LYX_GLUE_LENGTH_H
#define LYX_GLUE_LENGTH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxlength.h"

#include "LString.h"

class BufferParams;
class BufferView;


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


	/** If "data" is valid, the length represented by it is
	  stored into "result", if that is not 0. */
	friend bool isValidGlueLength(string const & data,
				      LyXGlueLength* result = 0);

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
///
bool isValidGlueLength(string const & data, LyXGlueLength * result);

#endif // LYXGLUELENGTH_H
