// -*- C++ -*-
#ifndef LYX_MANIP_H
#define LYX_MANIP_H

#include "support/LOstream.h"

using std::ostream;

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
ostream & operator<<(ostream & os, NewLineAndDepth_ const & nlad_)
{
	os << string(nlad_.depth_, ' ');
	return os;
}

#endif
