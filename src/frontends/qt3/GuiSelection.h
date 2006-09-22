// -*- C++ -*-
/**
 * \file qt3/GuiSelection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
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
 * The Qt3 version of the Selection.
 */
class GuiSelection: public lyx::frontend::Selection
{
public:
	GuiSelection() {}

	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool);

	docstring const get() const;

	void put(docstring const & str);
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
