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
#include "lengthcommon.h"

#include "Lsstream.h"


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

	buffer << len_.value();

	if (plus_.zero() && minus_.zero()) {
		buffer << unit_name[len_.unit()];
		return buffer.str().c_str();
	}
 
	// just len and plus
	if (minus_.zero()) {
		if (len_.unit() != plus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << "+" << plus_.value();
		buffer << unit_name[plus_.unit()];
		return buffer.str().c_str();
	}
 
	// just len and minus
	if (plus_.zero()) {
		if (len_.unit() != minus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << "-" << minus_.value();
		buffer << unit_name[minus_.unit()];
		return buffer.str().c_str();
	}

	// ok, len, plus AND minus
 
	// len+-
	if (minus_ == plus_) {
		if (len_.unit() != minus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << "+-" << minus_.value();
		buffer << unit_name[minus_.unit()];
		return buffer.str().c_str();
	}
 
	// this is so rare a case, why bother minimising units ?

	buffer << unit_name[len_.unit()];
	buffer << "+" << plus_.value() << unit_name[plus_.unit()];
	buffer << "-" << minus_.value() << unit_name[minus_.unit()];
 
	return buffer.str().c_str();
}


string const LyXGlueLength::asLatexString() const
{
	ostringstream buffer;

	buffer << len_.value() << unit_name[len_.unit()];
 
	if (!plus_.zero())
		buffer << " plus " << plus_.value() << unit_name[plus_.unit()];
	if (!minus_.zero())
		buffer << " minus " << minus_.value() << unit_name[minus_.unit()];
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
