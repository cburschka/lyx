#include <config.h>

#include "math_mathmlstream.h"
#include "math_inset.h"
#include "math_extern.h"
#include "debug.h"


MathMLStream::MathMLStream(std::ostream & os)
	: os_(os), tab_(0), line_(0), lastchar_(0)
{}


MathMLStream & MathMLStream::operator<<(MathInset const * p)
{
	if (p)
		p->mathmlize(*this);
	else
		lyxerr << "MathMLStream::operator<<(NULL) called\n";
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
	if (p)
		p->maplize(*this);
	else
		lyxerr << "MathMLStream::operator<<(NULL) called\n";
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
	if (p)
		p->octavize(*this);
	else
		lyxerr << "MathMLStream::operator<<(NULL) called\n";
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
	if (p)
		p->normalize(*this);
	else
		lyxerr << "MathMLStream::operator<<(NULL) called\n";
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
	: buffer(buffer_), os(os_), fragile(fragile_), line_(0)
{}


WriteStream::WriteStream(std::ostream & os_)
	: buffer(0), os(os_), fragile(false), line_(0)
{}


WriteStream & WriteStream::operator<<(MathInset const * p)
{
	if (p)
		p->write(*this);
	else
		lyxerr << "MathMLStream::operator<<(NULL) called\n";
	return *this;		
}


WriteStream & WriteStream::operator<<(MathArray const & ar)
{
	write(ar, *this);
	return *this;		
}


WriteStream & WriteStream::operator<<(string const & s)
{
	os << s;
	string::const_iterator cit = s.begin();
	string::const_iterator end = s.end();
	for ( ; cit != end ; ++cit) {
		if (*cit == '\n')
			++line_;
	}
	return *this;		
}


WriteStream & WriteStream::operator<<(char const * s)
{
	os << s;
	for ( ; *s ; ++s) {
		if (*s == '\n')
			++line_;
	}
	return *this;		
}


WriteStream & WriteStream::operator<<(char c)
{
	os << c;
	if (c == '\n')
		++line_;
	return *this;		
}


WriteStream & WriteStream::operator<<(int i)
{
	os << i;
	return *this;		
}


WriteStream & WriteStream::operator<<(unsigned int i)
{
	os << i;
	return *this;		
}
