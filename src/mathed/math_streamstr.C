#include <config.h>

#include "math_streamstr.h"
#include "math_mathmlstream.h"
#include "support/LOstream.h"
#include "support/lyxalgo.h"


WriteStream & operator<<(WriteStream & ws, string const & s)
{
	if (ws.pendingSpace()) {
		ws.os() << ' ';
		ws.pendingSpace(false);
	}
	ws.os() << s;
	ws.addlines(int(lyx::count(s.begin(), s.end(), '\n')));
	return ws;
}


NormalStream & operator<<(NormalStream & ns, string const & s)
{
	ns.os() << s;
	return ns;
}


MapleStream & operator<<(MapleStream & ms, string const & s)
{
	ms.os() << s;
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, string const & s)
{
	ms.os() << s;
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, string const & s)
{
	ms.os() << s;
	return ms;
}


OctaveStream & operator<<(OctaveStream & os, string const & s)
{
	os.os() << s;
	return os;
}
