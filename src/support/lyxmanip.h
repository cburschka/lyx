// -*- C++ -*-
/**
 * \file lyxmanip.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_MANIP_H
#define LYX_MANIP_H

#include "support/std_string.h"
#include "support/std_ostream.h"

///
struct NewLineAndDepth_ {
	int depth_;
};

///
inline
NewLineAndDepth_ newlineAndDepth(int n)
{
	NewLineAndDepth_ nlad_;
	nlad_.depth_ = n;
	return nlad_;
}

///
inline
std::ostream & operator<<(std::ostream & os, NewLineAndDepth_ const & nlad_)
{
	os << string(nlad_.depth_, ' ');
	return os;
}

#endif
