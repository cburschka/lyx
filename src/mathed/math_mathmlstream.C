
#ifdef __GNUG__
#pragma implementation 
#endif

#include <config.h>

#include "math_mathmlstream.h"
#include "math_inset.h"
#include "math_extern.h"
#include "debug.h"
#include "support/lyxalgo.h"
#include "support/LOstream.h"


using std::ostream;
using std::strlen;

namespace {

	bool isAlpha(char c) 
	{
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
	}

}

WriteStream::WriteStream(ostream & os, bool fragile, bool latex)
	: os_(os), fragile_(fragile), firstitem_(false), latex_(latex),
	  pendingspace_(false), line_(0)
{}


WriteStream::WriteStream(ostream & os)
	: os_(os), fragile_(false), firstitem_(false), latex_(false),
	  pendingspace_(false), line_(0)
{}


WriteStream::~WriteStream()
{
	if (pendingspace_)
		os_ << ' ';
}


void WriteStream::addlines(unsigned int n)
{
	line_ += n;
}


void WriteStream::pendingSpace(bool how)
{
	pendingspace_ = how;
}


WriteStream & operator<<(WriteStream & ws, MathAtom const & at)
{
	at->write(ws);
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
	ws.addlines(int(lyx::count(s, s + strlen(s), '\n')));
	return ws;
}


WriteStream & operator<<(WriteStream & ws, char c)
{
	if (ws.pendingSpace()) {
		if (isAlpha(c))
			ws.os() << ' ';
		ws.pendingSpace(false);
	}
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


//////////////////////////////////////////////////////////////////////


MathMLStream::MathMLStream(ostream & os)
	: os_(os), tab_(0), line_(0), lastchar_(0)
{}


MathMLStream & operator<<(MathMLStream & ms, MathAtom const & at)
{
	at->mathmlize(ms);
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


MapleStream & operator<<(MapleStream & ms, MathAtom const & at)
{
	at->maplize(ms);
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


MathematicaStream & operator<<(MathematicaStream & ms, MathAtom const & at)
{
	at->mathematicize(ms);
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, MathArray const & ar)
{
	mathematicize(ar, ms);
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, char const * s)
{
	ms.os() << s;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, char c)
{
	ms.os() << c;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, int i)
{
	ms.os() << i;
	return ms;
}



//////////////////////////////////////////////////////////////////////


OctaveStream & operator<<(OctaveStream & ns, MathAtom const & at)
{
	at->octavize(ns);
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


NormalStream & operator<<(NormalStream & ns, MathAtom const & at)
{
	at->normalize(ns);
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


NormalStream & operator<<(NormalStream & ns, int i)
{
	ns.os() << i;
	return ns;
}



//////////////////////////////////////////////////////////////////////

