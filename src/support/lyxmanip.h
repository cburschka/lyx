// -*- C++ -*-
#ifndef LYX_MANIP_H
#define LYX_MANIP_H

#include <iomanip>

///
inline
ostream & newlineAndDepth_helper(ostream & s, int n)
{
	return s << '\n' << string(n, ' ');
}
///
inline
omanip<int> newlineAndDepth(int n)
{
	return omanip<int>(newlineAndDepth_helper, n);
}

#endif
