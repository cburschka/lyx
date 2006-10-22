// -*- C++ -*-
/**
 * \file MathExtern.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_EXTERN_H
#define MATH_EXTERN_H

#include "support/docstring.h"
#include <string>

namespace lyx {

class NormalStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathStream;
class OctaveStream;
class WriteStream;
class MathArray;

void write(MathArray const &, WriteStream &);
void normalize(MathArray const &, NormalStream &);
void maple(MathArray const &, MapleStream &);
void maxima(MathArray const &, MaximaStream &);
void mathematica(MathArray const &, MathematicaStream &);
void mathmlize(MathArray const &, MathStream &);
void octave(MathArray const &, OctaveStream &);

bool extractNumber(MathArray const & ar, int & i);
bool extractNumber(MathArray const & ar, double & i);

MathArray pipeThroughExtern(std::string const & language,
	docstring const & extra, MathArray const & ar);


} // namespace lyx

#endif
