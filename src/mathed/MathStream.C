/**
 * \file MathStream.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathStream.h"
#include "MathMLStream.h"


namespace lyx {

using std::string;


WriteStream & operator<<(WriteStream & ws, string const & s)
{
	ws << s.c_str();
	return ws;
}


NormalStream & operator<<(NormalStream & ns, string const & s)
{
	// FIXME UNICODE
	ns.os() << from_utf8(s);
	return ns;
}


MapleStream & operator<<(MapleStream & ms, string const & s)
{
	// FIXME UNICODE
	ms.os() << from_utf8(s);
	return ms;
}


MaximaStream & operator<<(MaximaStream & ms, string const & s)
{
	// FIXME UNICODE
	ms.os() << from_utf8(s);
	return ms;
}


MathematicaStream & operator<<(MathematicaStream & ms, string const & s)
{
	// FIXME UNICODE
	ms.os() << from_utf8(s);
	return ms;
}


MathMLStream & operator<<(MathMLStream & ms, string const & s)
{
	// FIXME UNICODE
	ms.os() << from_utf8(s);
	return ms;
}


OctaveStream & operator<<(OctaveStream & os, string const & s)
{
	// FIXME UNICODE
	os.os() << from_utf8(s);
	return os;
}


} // namespace lyx
