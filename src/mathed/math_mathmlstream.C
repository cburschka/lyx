
#include "math_inset.h"
#include "math_mathmlstream.h"


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



//////////////////////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////////////////////


NormalStream & NormalStream::operator<<(MathInset const * p)
{
	p->writeNormal(*this);
	return *this;		
}


NormalStream & NormalStream::operator<<(MathArray const & ar)
{
	ar.writeNormal(*this);
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


MathWriteInfo::MathWriteInfo
		(Buffer const * buffer_, std::ostream & os_, bool fragile_)
	: buffer(buffer_), os(os_), fragile(fragile_)
{}


MathWriteInfo::MathWriteInfo(std::ostream & os_)
	: buffer(0), os(os_), fragile(false)
{}


MathWriteInfo & MathWriteInfo::operator<<(MathInset const * p)
{
	p->write(*this);
	return *this;		
}


MathWriteInfo & MathWriteInfo::operator<<(MathArray const & ar)
{
	ar.write(*this);
	return *this;		
}


MathWriteInfo & MathWriteInfo::operator<<(char const * s)
{
	os << s;
	return *this;		
}


MathWriteInfo & MathWriteInfo::operator<<(char c)
{
	os << c;
	return *this;		
}


