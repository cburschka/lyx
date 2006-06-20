// -*- C++ -*-
/**
 * \file xforms/GuiImplementation.h
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

#include "xscreen.h"
#include "XWorkArea.h"

#include "GuiClipboard.h"
#include "GuiWorkArea.h"

#include <boost/shared_ptr.hpp>

#include <map>

namespace lyx {
namespace frontend {

typedef XScreen FScreen;
typedef XWorkArea FWorkArea;

/**
 * The Gui class is the interface to all XForms components.
 */
class GuiImplementation: public lyx::frontend::Gui
{
public:
	GuiImplementation(LyXView & owner): owner_(owner)
	{
	}

	virtual ~GuiImplementation()
	{
	}

	lyx::frontend::Clipboard& clipboard()
	{
		return *clipboard_;
	}

	int newWorkArea(int w, int h)
	{
		old_work_area_.reset(new FWorkArea(owner_, w, h));
		old_screen_.reset(new FScreen(*old_work_area_.get()));
		work_area_.reset(new GuiWorkArea(owner_, w, h, old_screen_.get(), old_work_area_.get()));
		clipboard_.reset(new GuiClipboard(old_work_area_.get()));
		guiCursor().connect(work_area_.get());
		return 0;
	}

	lyx::frontend::WorkArea& workArea(int id)
	{
		return *work_area_;
	}

	void destroyWorkArea(int id)
	{
		clipboard_.reset();
		work_area_.reset();
		old_work_area_.reset();
		old_screen_.reset();
	}

private:
	///
	boost::shared_ptr<GuiClipboard> clipboard_;
	///
	boost::shared_ptr<GuiWorkArea> work_area_;
	///
	boost::shared_ptr<FWorkArea> old_work_area_;
	///
	boost::shared_ptr<FScreen> old_screen_;
	///
	LyXView & owner_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_IMPLEMENTATION_H
