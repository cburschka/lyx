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

#include "lyxlength.h"
#include "lengthcommon.h"

#include "Lsstream.h"

#include <cstdlib>


LyXLength::LyXLength()
	: val_(0), unit_(LyXLength::PT)
{}


LyXLength::LyXLength(double v, LyXLength::UNIT u)
	: val_(v), unit_(u)
{}


LyXLength::LyXLength(string const & data)
	: val_(0), unit_(LyXLength::PT)
{
	LyXLength tmp;
	
	if (!isValidLength (data, &tmp))
		return; // should raise an exception

	val_  = tmp.val_;
	unit_ = tmp.unit_;
}


string const LyXLength::asString() const
{
	ostringstream buffer;
	buffer << val_ << unit_name[unit_]; // setw?
	return buffer.str().c_str();
}


string const LyXLength::asLatexString() const
{
	ostringstream buffer;
	switch(unit_) {
	case PW:
	case PE:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\columnwidth";
	    break;
	case PP:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\pagewidth";
	    break;
	case PL:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\linewidth";
	    break;
	default:
	    buffer << val_ << unit_name[unit_]; // setw?
	    break;
	}
	return buffer.str().c_str();
}


double LyXLength::value() const
{
	return val_;
}


LyXLength::UNIT LyXLength::unit() const
{
	return unit_;
}


void LyXLength::value(double v)
{
	val_ = v;
}


void LyXLength::unit(LyXLength::UNIT u)
{
	unit_ = u;
}


bool LyXLength::zero() const 
{
	return val_ == 0.0;
}


bool operator==(LyXLength const & l1, LyXLength const & l2)
{
	return l1.value() == l2.value() && l1.unit() == l2.unit();
}


bool operator!=(LyXLength const & l1, LyXLength const & l2)
{
	return !(l1 == l2);
}

