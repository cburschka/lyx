#ifndef MATH_EXTERN_H
#define MATH_EXTERN_H

class NormalStream;
class MapleStream;
class MathMLStream;
class OctaveStream;
class WriteStream;
class MathArray;

void write(MathArray const &, WriteStream &);
void normalize(MathArray const &, NormalStream &);
void maplize(MathArray const &, MapleStream &);
void mathmlize(MathArray const &, MathMLStream &);
void octavize(MathArray const &, OctaveStream &);

#endif
