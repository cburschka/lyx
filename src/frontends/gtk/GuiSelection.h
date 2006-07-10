// -*- C++ -*-
/**
 * \file gtk/Selection.h
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

#include "GWorkArea.h"

namespace lyx {
namespace frontend {

/**
 * The GTK version of the Selection.
 */
class GuiSelection: public lyx::frontend::Selection
{
public:
	GuiSelection(GWorkArea * work_area)
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

	std::string const get() const;

	void put(std::string const & str);
	//@}

private:
	GWorkArea * old_work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
