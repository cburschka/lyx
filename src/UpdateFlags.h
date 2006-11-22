// -*- C++ -*-
/**
 * \file UpdateFlags.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author The Denmark Cowboys
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef UPDATE_FLAGS_H
#define UPDATE_FLAGS_H

namespace lyx {

namespace Update {
	enum flags {
		None = 0,
		FitCursor = 1,
		Force = 2,
		SinglePar = 4,
		MultiParSel = 8
	};

inline flags operator|(flags const f, flags const g)
{
	return static_cast<flags>(int(f) | int(g));
}

inline flags operator&(flags const f, flags const g)
{
	return static_cast<flags>(int(f) & int(g));
}

} // namespace

} // namespace lyx
#endif
