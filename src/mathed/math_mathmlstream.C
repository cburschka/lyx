#include <config.h>

#include "math_mathmlstream.h"
#include "math_inset.h"
#include "math_extern.h"
#include "debug.h"
#include "support/lyxalgo.h"


using std::ostream;
using std::strlen;


MathMLStream::MathMLStream(ostream & os)
	: os_(os), tab_(0), line_(0), lastchar_(0)
{}


MathMLStream & operator<<(MathMLStream & ms, MathInset const * p)
{
	if (p)
		p->mathmlize(ms);
	else
		lyxerr << "operator<<(MathMLStream, NULL) called\n";
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, MathArray const & ar)
{
	mathmlize(ar, ms);
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, MTag const & t)
{
	++ms.tab();
	ms.cr();
	ms.os() << '<' << t.tag_ << '>';
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, ETag const & t)
{
	ms.cr();
	if (ms.tab() > 0)
		--ms.tab();
	ms.os() << "</" << t.tag_ << '>';
	return ms;
}


void MathMLStream::cr()
{
	os() << '\n';
	for (int i = 0; i < tab(); ++i)
		os() << ' ';
}



//////////////////////////////////////////////////////////////////////


MapleStream & operator<<(MapleStream & ms, MathInset const * p)
{
	if (p)
		p->maplize(ms);
	else
		lyxerr << "operator<<(MapleStream, NULL) called\n";
	return ms;
}


MapleStream & operator<<(MapleStream & ms, MathArray const & ar)
{
	maplize(ar, ms);
	return ms;
}


MapleStream & operator<<(MapleStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MapleStream & operator<<(MapleStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MapleStream & operator<<(MapleStream & ms, int i)
{
	ms.os() << i;
	return ms;
}


//////////////////////////////////////////////////////////////////////


OctaveStream & operator<<(OctaveStream & ns, MathInset const * p)
{
	if (p)
		p->octavize(ns);
	else
		lyxerr << "operator<<(OctaveStream, NULL) called\n";
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, MathArray const & ar)
{
	octavize(ar, ns);
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, char const * s)
{
	ns.os() << s;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, char c)
{
	ns.os() << c;
	return ns;
}


OctaveStream & operator<<(OctaveStream & ns, int i)
{
	ns.os() << i;
	return ns;
}


//////////////////////////////////////////////////////////////////////


NormalStream & operator<<(NormalStream & ns, MathInset const * p)
{
	if (p)
		p->normalize(ns);
	else
		lyxerr << "operator<<(NormalStream, NULL) called\n";
	return ns;
}


NormalStream & operator<<(NormalStream & ns, MathArray const & ar)
{
	normalize(ar, ns);
	return ns;
}


NormalStream & operator<<(NormalStream & ns, char const * s)
{
	ns.os() << s;
	return ns;
}


NormalStream & operator<<(NormalStream & ns, char c)
{
	ns.os() << c;
	return ns;
}



//////////////////////////////////////////////////////////////////////


WriteStream::WriteStream(ostream & os, bool fragile, bool latex)
	: os_(os), fragile_(fragile), latex_(latex), firstitem_(false), line_(0)
{}


WriteStream::WriteStream(ostream & os)
	: os_(os), fragile_(false), latex_(false), firstitem_(false), line_(0)
{}


void WriteStream::addlines(unsigned int n)
{
	line_ += n;
}


WriteStream & operator<<(WriteStream & ws, MathInset const * p)
{
	if (p)
		p->write(ws);
	else
		lyxerr << "operator<<(WriteStream, NULL) called\n";
	return ws;
}


WriteStream & operator<<(WriteStream & ws, MathArray const & ar)
{
	write(ar, ws);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, char const * s)
{
	ws.os() << s;
	ws.addlines(int(lyx::count(s, s+strlen(s), '\n')));
	return ws;
}


WriteStream & operator<<(WriteStream & ws, char c)
{
	ws.os() << c;
	if (c == '\n')
		ws.addlines(1);
	return ws;
}


WriteStream & operator<<(WriteStream & ws, int i)
{
	ws.os() << i;
	return ws;
}


WriteStream & operator<<(WriteStream & ws, unsigned int i)
{
	ws.os() << i;
	return ws;
}
