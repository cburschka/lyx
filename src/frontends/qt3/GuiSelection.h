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

#include "QWorkArea.h"

namespace lyx {
namespace frontend {

/**
 * The Qt3 version of the Selection.
 */
class GuiSelection: public lyx::frontend::Selection
{
public:
	GuiSelection(QWorkArea * work_area)
		: old_work_area_(work_area)
	{
	}

	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool own)
	{
		old_work_area_->haveSelection(own);
	}

	docstring const get() const;

	void put(docstring const & str);
	//@}

private:
	QWorkArea * old_work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
