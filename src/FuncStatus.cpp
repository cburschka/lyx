/**
 * \file FuncStatus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FuncStatus.h"


namespace lyx {

FuncStatus::FuncStatus()
	: v_(OK)
{
}


void FuncStatus::clear()
{
	v_ = OK;
	message_.erase();
}


void FuncStatus::operator|=(FuncStatus const & f)
{
	v_ |= f.v_;
	if (!f.message_.empty())
		message_ = f.message_;
}


void FuncStatus::unknown(bool b)
{
	if (b)
		v_ |= UNKNOWN;
	else
		v_ &= ~UNKNOWN;
}



bool FuncStatus::unknown() const
{
	return (v_ & UNKNOWN);
}


void FuncStatus::setEnabled(bool b)
{
	if (b)
		v_ &= ~DISABLED;
	else
		v_ |= DISABLED;
}


bool FuncStatus::enabled() const
{
	return !(v_ & DISABLED);
}


void FuncStatus::setOnOff(bool b)
{
	v_ |= (b ? ON : OFF);
}


bool FuncStatus::onoff(bool b) const
{
	if (b)
		return (v_ & ON);
	else
		return (v_ & OFF);
}


void FuncStatus::message(docstring const & m)
{
	message_ = m;
}


docstring const & FuncStatus::message() const
{
	return message_;
}


} // namespace lyx
