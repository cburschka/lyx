#ifndef MATH_EXTERN_H
#define MATH_EXTERN_H


#include "LString.h"

class NormalStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathMLStream;
class OctaveStream;
class WriteStream;
class MathArray;

void write(MathArray const &, WriteStream &);
void normalize(MathArray const &, NormalStream &);
void maple(MathArray const &, MapleStream &);
void maxima(MathArray const &, MaximaStream &);
void mathematica(MathArray const &, MathematicaStream &);
void mathmlize(MathArray const &, MathMLStream &);
void octave(MathArray const &, OctaveStream &);

bool extractNumber(MathArray const & ar, int & i);
bool extractNumber(MathArray const & ar, double & i);

MathArray pipeThroughExtern(string const & lang, string const & extra,
	MathArray const & ar);

#endif
