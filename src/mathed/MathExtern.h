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

#include "support/strfwd.h"

namespace lyx {

class HtmlStream;
class NormalStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathStream;
class OctaveStream;
class WriteStream;
class MathData;

void write(MathData const &, WriteStream &);
void htmlize(MathData const &, HtmlStream &);
void normalize(MathData const &, NormalStream &);
void maple(MathData const &, MapleStream &);
void maxima(MathData const &, MaximaStream &);
void mathematica(MathData const &, MathematicaStream &);
void mathmlize(MathData const &, MathStream &);
void octave(MathData const &, OctaveStream &);

bool extractNumber(MathData const & ar, int & i);
bool extractNumber(MathData const & ar, double & i);

/// Write \p s (which may contain math or text contents in LaTeX syntax) to \p os
void writeString(docstring const & s, WriteStream & os);

MathData pipeThroughExtern(std::string const & language,
	docstring const & extra, MathData const & ar);


} // namespace lyx

#endif
