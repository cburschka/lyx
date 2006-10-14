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

#ifndef BASE_SELECTION_H
#define BASE_SELECTION_H

#include "support/docstring.h"

namespace lyx {
namespace frontend {

/**
 * A Selection class manages the selection.
 */
class Selection
{
public:
	virtual ~Selection() {}

	/// Tell the window system whether we have a selection.
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
	 * Does nothing on systems that don't have a selection.
	 * This should be called whenever some text is highlighted.
	 */
	virtual void put(docstring const &) = 0;
};

} // namespace frontend
} // namespace lyx

/// Implementation is in Application.C
extern lyx::frontend::Selection & theSelection();

#endif // BASE_SELECTION_H
