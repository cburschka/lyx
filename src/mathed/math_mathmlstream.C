
#include "support/LOstream.h"
#include "math_inset.h"
#include "math_mathmlstream.h"


MathMLStream & MathMLStream::operator<<(MathInset const * p)
{
	p->mathmlize(*this);
	return *this;		
}


MathMLStream & MathMLStream::operator<<(MathArray const & ar)
{
	ar.mathmlize(*this);
	return *this;		
}


MathMLStream & MathMLStream::operator<<(char const * s)
{
	os_ << s;
	return *this;		
}


MathMLStream & MathMLStream::operator<<(char c)
{
	os_ << c;
	return *this;		
}




MapleStream & MapleStream::operator<<(MathInset const * p)
{
	p->maplize(*this);
	return *this;		
}


MapleStream & MapleStream::operator<<(MathArray const & ar)
{
	ar.maplize(*this);
	return *this;		
}


MapleStream & MapleStream::operator<<(char const * s)
{
	os_ << s;
	return *this;		
}


MapleStream & MapleStream::operator<<(char c)
{
	os_ << c;
	return *this;		
}




OctaveStream & OctaveStream::operator<<(MathInset const * p)
{
	p->octavize(*this);
	return *this;		
}


OctaveStream & OctaveStream::operator<<(MathArray const & ar)
{
	ar.octavize(*this);
	return *this;		
}


OctaveStream & OctaveStream::operator<<(char const * s)
{
	os_ << s;
	return *this;		
}


OctaveStream & OctaveStream::operator<<(char c)
{
	os_ << c;
	return *this;		
}

