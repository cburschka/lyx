#ifndef MATH_STREAMSTR_H
#define MATH_STREAMSTR_H

#include "LString.h"
#include "math_mathmlstream.h"

//
// writing strings directly
//

inline WriteStream & operator<<(WriteStream & ws, string const & s)
{
	ws << s.c_str();
	return ws;
}

#endif
