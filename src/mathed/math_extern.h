#ifndef MATH_EXTERN_H
#define MATH_EXTERN_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class NormalStream;
class MapleStream;
class MathematicaStream;
class MathMLStream;
class OctaveStream;
class WriteStream;
class MathArray;

void write(MathArray const &, WriteStream &);
void normalize(MathArray const &, NormalStream &);
void maplize(MathArray const &, MapleStream &);
void mathematicize(MathArray const &, MathematicaStream &);
void mathmlize(MathArray const &, MathMLStream &);
void octavize(MathArray const &, OctaveStream &);

bool extractNumber(MathArray const & ar, int & i);
bool extractNumber(MathArray const & ar, double & i);

MathArray pipeThroughExtern(string const & lang, string const & extra,
	MathArray const & ar);

#endif
