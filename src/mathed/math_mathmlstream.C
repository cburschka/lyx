
#include "math_inset.h"
#include "math_mathmlstream.h"
#include "math_extern.h"


MathMLStream::MathMLStream(std::ostream & os)
	: os_(os), tab_(0), line_(0)
{}


MathMLStream & MathMLStream::operator<<(MathInset const * p)
{
	p->mathmlize(*this);
	return *this;		
}


MathMLStream & MathMLStream::operator<<(MathArray const & ar)
{
	mathmlize(ar, *this);
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


MathMLStream & MathMLStream::operator<<(MTag const & t)
{
	++tab_;
	cr();
	os_ << '<' << t.tag_ << '>';
	return *this;		
}


MathMLStream & MathMLStream::operator<<(ETag const & t)
{
	cr();
	if (tab_ > 0)
		--tab_;
	os_ << "</" << t.tag_ << '>';
	return *this;		
}


void MathMLStream::cr()
{
	os_ << '\n';
	for (int i = 0; i < tab_; ++i)
		os_ << ' ';
}



//////////////////////////////////////////////////////////////////////


MapleStream & MapleStream::operator<<(MathInset const * p)
{
	p->maplize(*this);
	return *this;		
}


MapleStream & MapleStream::operator<<(MathArray const & ar)
{
	maplize(ar, *this);
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


MapleStream & MapleStream::operator<<(int i)
{
	os_ << i;
	return *this;		
}


//////////////////////////////////////////////////////////////////////


OctaveStream & OctaveStream::operator<<(MathInset const * p)
{
	p->octavize(*this);
	return *this;		
}


OctaveStream & OctaveStream::operator<<(MathArray const & ar)
{
	octavize(ar, *this);
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


//////////////////////////////////////////////////////////////////////


NormalStream & NormalStream::operator<<(MathInset const * p)
{
	p->normalize(*this);
	return *this;		
}


NormalStream & NormalStream::operator<<(MathArray const & ar)
{
	normalize(ar, *this);
	return *this;		
}


NormalStream & NormalStream::operator<<(char const * s)
{
	os_ << s;
	return *this;		
}


NormalStream & NormalStream::operator<<(char c)
{
	os_ << c;
	return *this;		
}



//////////////////////////////////////////////////////////////////////


WriteStream::WriteStream
		(Buffer const * buffer_, std::ostream & os_, bool fragile_)
	: buffer(buffer_), os(os_), fragile(fragile_)
{}


WriteStream::WriteStream(std::ostream & os_)
	: buffer(0), os(os_), fragile(false)
{}


WriteStream & WriteStream::operator<<(MathInset const * p)
{
	p->write(*this);
	return *this;		
}


WriteStream & WriteStream::operator<<(MathArray const & ar)
{
	write(ar, *this);
	return *this;		
}


WriteStream & WriteStream::operator<<(char const * s)
{
	os << s;
	return *this;		
}


WriteStream & WriteStream::operator<<(char c)
{
	os << c;
	return *this;		
}


