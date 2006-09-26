// -*- C++ -*-
/**
 * \file qt3/Gui.h
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

#include "QtView.h"
#include "qscreen.h"
#include "QWorkArea.h"

#include "GuiWorkArea.h"

#include "BufferView.h"

#include <boost/shared_ptr.hpp>

namespace lyx {
namespace frontend {

typedef QtView FView;
typedef QScreen FScreen;
typedef QWorkArea FWorkArea;

/**
 * The Gui class is the interface to all Qt3 components.
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

	int newView(unsigned int /*w*/, unsigned int /*h*/)
	{
		view_.reset(new FView);
		return 0;
	}


	LyXView& view(int /*id*/)
	{
		return *view_;
	}


	void destroyView(int /*id*/)
	{
		view_.reset();
	}

	int newWorkArea(unsigned int w, unsigned int h, int /*view_id*/)
	{
		old_work_area_.reset(new FWorkArea(*view_.get(), w, h));
		old_screen_.reset(new FScreen(*old_work_area_.get()));
		work_area_.reset(new GuiWorkArea(old_screen_.get(), old_work_area_.get()));

		// FIXME BufferView creation should be independant of WorkArea creation
		buffer_views_[0].reset(new BufferView);
		work_area_->setBufferView(buffer_views_[0].get());
		view_->setWorkArea(work_area_.get());
		return 0;
	}

	lyx::frontend::WorkArea& workArea(int /*id*/)
	{
		return *work_area_;
	}

	void destroyWorkArea(int /*id*/)
	{
		work_area_.reset();
		old_work_area_.reset();
		old_screen_.reset();
	}

private:
	///
	boost::shared_ptr<GuiWorkArea> work_area_;
	///
	boost::shared_ptr<FView> view_;
	///
	boost::shared_ptr<FWorkArea> old_work_area_;
	///
	boost::shared_ptr<FScreen> old_screen_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_IMPLEMENTATION_H
