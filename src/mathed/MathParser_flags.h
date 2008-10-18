// -*- C++ -*-
/**
 * \file mathparser_flags.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATHPARSER_FLAGS_H
#define MATHPARSER_FLAGS_H

namespace lyx {

namespace Parse {

enum flags {
	/// Parse normally.
	NORMAL = 0x00,
	/// Start parsing in text mode.
	TEXTMODE = 0x01,
	/// Parse verbatim.
	VERBATIM = 0x02,
	/// Quiet operation (no warnigs or errors).
	QUIET = 0x04
};


inline flags operator|(flags const f, flags const g)
{
	return static_cast<flags>(int(f) | int(g));
}


inline flags & operator|=(flags & f, flags g)
{
	return f = static_cast<flags>(int(f) | int(g));
}


inline flags operator&(flags const f, flags const g)
{
	return static_cast<flags>(int(f) & int(g));
}

} // namespace Parse

} // namespace lyx
#endif
