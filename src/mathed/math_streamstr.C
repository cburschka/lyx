#include <config.h>
#include <algorithm>

#include "math_mathmlstream.h"
#include "math_streamstr.h"


WriteStream & operator<<(WriteStream & ws, string const & s)
{
	ws.os() << s;
	ws.line() += std::count(s.begin(), s.end(), '\n');
	return ws;
}
