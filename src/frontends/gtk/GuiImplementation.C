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

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GuiImplementation.h"

#include "GView.h"
#include "BufferView.h"

// FIXME: defined in X.h, spuriously pulled in by some gui headers
#undef CursorShape

namespace lyx {
namespace frontend {

int GuiImplementation::newView(unsigned int /*w*/, unsigned int /*h*/)
{
	view_.reset(new GView);
	return 0;
}


int GuiImplementation::newWorkArea(unsigned int w, unsigned int h, int /*view_id*/)
{
	old_work_area_.reset(new GWorkArea(*view_.get(), w, h));
	old_screen_.reset(new GScreen(*old_work_area_.get()));
	work_area_.reset(new GuiWorkArea(old_screen_.get(), old_work_area_.get()));

	// FIXME BufferView creation should be independant of WorkArea creation
	buffer_views_[0].reset(new BufferView(view_.get()));
	work_area_->setBufferView(buffer_views_[0].get());
	view_->setWorkArea(work_area_.get());
	return 0;
}


void GuiImplementation::destroyWorkArea(int /*id*/)
{
	work_area_.reset();
	old_work_area_.reset();
	old_screen_.reset();
}

} // namespace frontend
} // namespace lyx
