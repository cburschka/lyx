#ifndef MATH_STREAMSTR_H
#define MATH_STREAMSTR_H

#include "LString.h"

class WriteStream;

//
// writing strings directly
//

WriteStream & operator<<(WriteStream & ws, string const & s);

#endif
