/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxgluelength.h"

#include "Lsstream.h"

namespace {
// this is now here and in lyxlenght.h

int const num_units = LyXLength::UNIT_NONE;

// I am not sure if "mu" should be possible to select (Lgb)
char const * unit_name[num_units] = { "sp", "pt", "bp", "dd",
				      "mm", "pc", "cc", "cm",
				      "in", "ex", "em", "mu",
				      "%",  "c%", "p%", "l%" };

}


LyXGlueLength::LyXGlueLength(LyXLength const & len)
	: len_(len)
{}


LyXGlueLength::LyXGlueLength(LyXLength const & len, LyXLength const & plus,
		LyXLength const & minus)
	: len_(len), plus_(plus), minus_(minus)
{}


LyXGlueLength::LyXGlueLength(string const & data)
{
	isValidGlueLength(data, this);
}


string const LyXGlueLength::asString() const
{
	ostringstream buffer;

	if (plus_.value() != 0.0)
		if (minus_.value() != 0.0)
			if (len_.unit() == plus_.unit() && len_.unit() == minus_.unit())
				if (plus_.value() == minus_.value())
					buffer << len_.value() << "+-"
					       << plus_.value() << unit_name[len_.unit()];
				else
					buffer << len_.value()
					       << '+' << plus_.value()
					       << '-' << minus_.value()
					       << unit_name[len_.unit()];
			else
				if (plus_.unit() == minus_.unit()
				    && plus_.value() == minus_.value())
					buffer << len_.value() << unit_name[len_.unit()]
					       << "+-" << plus_.value()
					       << unit_name[plus_.unit()];
	
				else
					buffer << len_.value() << unit_name[len_.unit()]
					       << '+' << plus_.value()
					       << unit_name[plus_.unit()]
					       << '-' << minus_.value()
					       << unit_name[minus_.unit()];
		else
			if (len_.unit() == plus_.unit())
				buffer << len_.value() << '+' << plus_.value()
				       << unit_name[len_.unit()];
			else
				buffer << len_.value() << unit_name[len_.unit()]
				       << '+' << plus_.value()
				       << unit_name[plus_.unit()];
	
	else
		if (minus_.value() != 0.0)
			if (len_.unit() == minus_.unit())
				buffer << len_.value() << '-' << minus_.value()
				       << unit_name[len_.unit()];
	
			else
				buffer << len_.value() << unit_name[len_.unit()]
				       << '-' << minus_.value()
				       << unit_name[minus_.unit()];
		else
			buffer << len_.value() << unit_name[len_.unit()];

	return buffer.str().c_str();
}


string const LyXGlueLength::asLatexString() const
{
	ostringstream buffer;

	if (plus_.value() != 0.0)
		if (minus_.value() != 0.0)
			buffer << len_.value() << unit_name[len_.unit()]
			       << " plus "
			       << plus_.value() << unit_name[plus_.unit()]
			       << " minus "
			       << minus_.value() << unit_name[minus_.unit()];
		else
			buffer << len_.value() << unit_name[len_.unit()]
			       << " plus "
			       << plus_.value() << unit_name[plus_.unit()];
	else
		if (minus_.value() != 0.0)
			buffer << len_.value() << unit_name[len_.unit()]
			       << " minus "
			       << minus_.value() << unit_name[minus_.unit()];
		else
			buffer << len_.value() << unit_name[len_.unit()];

	return buffer.str().c_str();
}


LyXLength const & LyXGlueLength::len() const
{
	return len_;
}


LyXLength const & LyXGlueLength::plus() const
{
	return plus_;
}


LyXLength const & LyXGlueLength::minus() const
{
	return minus_;
}


bool operator==(LyXGlueLength const & l1, LyXGlueLength const & l2)
{
	return l1.len() == l2.len()
	         && l1.plus() == l2.plus()
	         && l1.minus() == l2.minus();
}


bool operator!=(LyXGlueLength const & l1, LyXGlueLength const & l2)
{
	return !(l1 == l2);
}
