// -*- C++ -*-
#ifndef MATH_STREAMSTR_H
#define MATH_STREAMSTR_H

#include "LString.h"

class WriteStream;
class NormalStream;
class MapleStream;
class MathMLStream;
class OctaveStream;

//
// writing strings directly
//

WriteStream & operator<<(WriteStream & ws, string const & s);
NormalStream & operator<<(NormalStream & ns, string const & s);
MapleStream & operator<<(MapleStream & ms, string const & s);
MathMLStream & operator<<(MathMLStream & ms, string const & s);
OctaveStream & operator<<(OctaveStream & os, string const & s);
#endif
