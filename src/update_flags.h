// -*- C++ -*-
/**
 * \file update_flags.h
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
		/// No screen update is needed.
		None = 0,
		/// Recenter the screen around the cursor if is found outside the
		/// visible area.
		FitCursor = 1,
		/// Force a full screen metrics update.
		Force = 2,
		/// Try to rebreak only the current paragraph metrics.
		SinglePar = 4,
		/// Only the inset decorations need to be redrawn, no text metrics
		/// update is needed.
		Decoration = 8
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
