/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *          Copyright 2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FuncStatus.h"

FuncStatus::FuncStatus() : v_(OK)
{
}


FuncStatus & FuncStatus::clear ()
{
	v_ = OK;
	return *this;
}

void FuncStatus::operator |= (FuncStatus const & f)
{
	v_ |= f.v_;
}

FuncStatus & FuncStatus::unknown (bool b)
{
	if (b)
		v_ |= UNKNOWN;
	else
		v_ &= !UNKNOWN;
	return *this;
}


bool FuncStatus::unknown () const
{
	return (v_ & UNKNOWN);
}


FuncStatus & FuncStatus::disabled (bool b)
{
	if (b)
		v_ |= DISABLED;
	else
		v_ &= !DISABLED;
	return *this;
}


bool FuncStatus::disabled () const
{
	return (v_ & DISABLED);
}


void FuncStatus::setOnOff (bool b)
{
	v_ |= (b ? ON : OFF);
}


bool FuncStatus::onoff (bool b) const
{
	if (b)
		return (v_ & ON);
	else
		return (v_ & OFF);
}
