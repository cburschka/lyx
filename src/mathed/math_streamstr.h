// -*- C++ -*-
/**
 * \file math_streamstr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_STREAMSTR_H
#define MATH_STREAMSTR_H

#include <string>

class WriteStream;
class NormalStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathMLStream;
class OctaveStream;

//
// writing strings directly
//

WriteStream & operator<<(WriteStream & ws, std::string const & s);
NormalStream & operator<<(NormalStream & ns, std::string const & s);
MapleStream & operator<<(MapleStream & ms, std::string const & s);
MaximaStream & operator<<(MaximaStream & ms, std::string const & s);
MathematicaStream & operator<<(MathematicaStream & ms, std::string const & s);
MathMLStream & operator<<(MathMLStream & ms, std::string const & s);
OctaveStream & operator<<(OctaveStream & os, std::string const & s);
#endif
