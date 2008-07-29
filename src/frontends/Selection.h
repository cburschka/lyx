// -*- C++ -*-
/**
 * \file Selection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SELECTION_H
#define SELECTION_H

#include "support/strfwd.h"

namespace lyx {
namespace frontend {

/**
 * A Selection class manages the selection.
 */
class Selection
{
public:
	virtual ~Selection() {}

	/**
	 * Tell the window system whether we set or cleared our selection.
	 * This is a noop on systems that don't have a selection.
	 * This should be called by the kernel whenever a selection is
	 * created, changed or cleared.
	 * \param own
	 * If true: Tell that we got a valid selection.
	 * If false: Tell that we cleared our selection.
	 */
	virtual void haveSelection(bool) = 0;
	/**
	 * Get the X selection contents.
	 * This is a noop on systems that don't have a selection.
	 * The format is plain text.
	 * This should be called when the user presses the middle mouse
	 * button.
	 */
	virtual docstring const get() const = 0;
	/**
	 * Fill the X selection.
	 * The format is plain text.
	 * Does nothing on systems that don't have a selection.
	 * Since the X selection protocol is asyncronous this should never be
	 * called from the kernel.
	 * If another application requests the current selection a
	 * SelectionRequest XEvent will be triggered. The response of the
	 * frontend to this event should be to retrieve the current selection
	 * from the kernel and push it to X with this method.
	 */
	virtual void put(docstring const &) = 0;
	/**
	 * Is the X selection empty?
	 * This does always return true on systems that don't have a real
	 * selection.
	 */
	virtual bool empty() = 0;
};

} // namespace frontend

/// Implementation is in Application.cpp
extern frontend::Selection & theSelection();

} // namespace lyx


#endif // SELECTION_H
