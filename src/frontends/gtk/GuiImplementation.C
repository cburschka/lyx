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

#include "GuiImplementation.h"

#include "GView.h"
#include "BufferView.h"

namespace lyx {
namespace frontend {

int GuiImplementation::newView(unsigned int /*w*/, unsigned int /*h*/)
{
	view_.reset(new GView(*this));
	return 0;
}


int GuiImplementation::newWorkArea(unsigned int w, unsigned int h, int /*view_id*/)
{
	old_work_area_.reset(new GWorkArea(*view_.get(), w, h));
	old_screen_.reset(new GScreen(*old_work_area_.get()));
	work_area_.reset(new GuiWorkArea(old_screen_.get(), old_work_area_.get()));
	clipboard_.reset(new GuiClipboard(old_work_area_.get()));
	guiCursor().connect(work_area_.get());
	
	// FIXME BufferView creation should be independant of WorkArea creation
	buffer_views_[0].reset(new BufferView(view_.get(), work_area_.get()));
	work_area_->setBufferView(buffer_views_[0].get());
	view_->setBufferView(buffer_views_[0].get());
	return 0;
}


void GuiImplementation::destroyWorkArea(int /*id*/)
{
	clipboard_.reset();
	work_area_.reset();
	old_work_area_.reset();
	old_screen_.reset();
}

} // namespace frontend
} // namespace lyx
