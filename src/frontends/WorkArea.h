// -*- C++ -*-
/**
 * \file WorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_WORKAREA_H
#define BASE_WORKAREA_H

#include "frontends/KeyModifier.h"

namespace lyx {

namespace frontend {

/**
 * The work area class represents the widget that provides the view
 * onto a document. It owns the BufferView, and is responsible for
 * handing events back to its BufferView.
 */
class WorkArea
{
public:
	///
	virtual ~WorkArea() {}

	/// Update metrics if needed and schedule a paint event
	virtual void scheduleRedraw(bool update_metrics) = 0;

	/// close this work area.
	/// Slot for Buffer::closing signal.
	virtual void close() = 0;

	/// Update window titles of all users.
	virtual void updateWindowTitle() = 0;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
