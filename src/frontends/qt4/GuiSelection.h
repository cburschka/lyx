// -*- C++ -*-
/**
 * \file qt4/GuiSelection.h
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

#include "frontends/Selection.h"

namespace lyx {
namespace frontend {

/**
 * The Qt4 version of the Selection.
 */
class GuiSelection: public Selection
{
public:
	GuiSelection() : empty_(true) {}
	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool own);
	docstring const get() const;
	void put(docstring const & str);
	bool empty() const;
	//@}
private:
	/**
	 * Is the selection empty?
	 * Only used on systems that don't support a real selection to
	 * reflect the status of the internal selection of LyX.
	 * This is needed to emulate the X selection as far as possible.
	 */
	bool empty_;
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
