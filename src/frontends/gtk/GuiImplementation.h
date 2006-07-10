// -*- C++ -*-
/**
 * \file gtk/GuiImplementation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_IMPLEMENTATION_H
#define GUI_IMPLEMENTATION_H

#include "frontends/Gui.h"
#include "frontends/LyXView.h"

#include "GScreen.h"
#include "GWorkArea.h"

#include "GuiClipboard.h"
#include "GuiSelection.h"
#include "GuiWorkArea.h"

#include <boost/shared_ptr.hpp>

namespace lyx {
namespace frontend {

/**
 * The Gui class is the interface to all GTK components.
 */
class GuiImplementation: public lyx::frontend::Gui
{
public:
	GuiImplementation()
	{
	}

	virtual ~GuiImplementation()
	{
	}

	lyx::frontend::Clipboard & clipboard()
	{
		return clipboard_;
	}

	lyx::frontend::Selection & selection()
	{
		return *selection_;
	}

	int newView(unsigned int w, unsigned int h);

	LyXView & view(int /*id*/)
	{
		return *view_;
	}

	void destroyView(int /*id*/)
	{
		view_.reset();
	}

	int newWorkArea(unsigned int w, unsigned int h, int /*view_id*/);

	lyx::frontend::WorkArea & workArea(int /*id*/)
	{
		return *work_area_;
	}

	void destroyWorkArea(int /*id*/);

private:
	///
	GuiClipboard clipboard_;
	///
	boost::shared_ptr<GuiSelection> selection_;
	///
	boost::shared_ptr<GuiWorkArea> work_area_;
	///
	boost::shared_ptr<LyXView> view_;
	///
	boost::shared_ptr<GWorkArea> old_work_area_;
	///
	boost::shared_ptr<GScreen> old_screen_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_IMPLEMENTATION_H
