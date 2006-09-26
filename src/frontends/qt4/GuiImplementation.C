// -*- C++ -*-
/**
 * \file GuiImplementation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// This include must be declared before everything else because
// of boost/Qt/LyX clash...
#include "GuiView.h"

#include "GuiImplementation.h"
#include "GuiWorkArea.h"

#include "BufferView.h"

using boost::shared_ptr;

namespace lyx {
namespace frontend {

GuiImplementation::GuiImplementation(): max_view_id_(0), max_wa_id_(0)
{
}


int GuiImplementation::newView(unsigned int /*w*/, unsigned int /*h*/)
{
	size_t const id = max_view_id_;
	++max_view_id_;

	views_[id].reset(new GuiView());

	return id;
}


LyXView& GuiImplementation::view(int id)
{
	BOOST_ASSERT(views_.find(id) != views_.end());

	return *views_[id].get();
}


void GuiImplementation::destroyView(int id)
{
	views_.erase(id);
}


int GuiImplementation::newWorkArea(unsigned int w, unsigned int h, int view_id)
{
	size_t const id = max_wa_id_;
	++max_wa_id_;

	GuiView * view = views_[view_id].get();

	work_areas_[id].reset(new GuiWorkArea(w, h, *view));

	// FIXME BufferView creation should be independant of WorkArea creation
	buffer_views_[id].reset(new BufferView);
	work_areas_[id]->setBufferView(buffer_views_[id].get());
	view->setWorkArea(work_areas_[id].get());

	view->setCentralWidget(work_areas_[id].get());

	return id;
}


WorkArea& GuiImplementation::workArea(int id)
{
	BOOST_ASSERT(work_areas_.find(id) != work_areas_.end());

	return *work_areas_[id].get();
}


void GuiImplementation::destroyWorkArea(int id)
{
	work_areas_.erase(id);
}

} // namespace frontend
} // namespace lyx
