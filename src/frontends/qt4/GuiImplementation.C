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

#include "GuiImplementation.h"
#include "GuiWorkArea.h"
#include "GuiView.h"

using boost::shared_ptr;

namespace lyx {
namespace frontend {

GuiImplementation::GuiImplementation(GuiView & owner): owner_(owner), max_id_(0)
{
}


Clipboard& GuiImplementation::clipboard()
{
	return clipboard_;
}


int GuiImplementation::newWorkArea(int w, int h)
{
	size_t const id = max_id_;
	++max_id_;
	work_areas_[id].reset(new GuiWorkArea(owner_, w, h));
	return id;
}

WorkArea& GuiImplementation::workArea(int id)
{
	BOOST_ASSERT(work_areas_.find(id) != work_areas_.end());

	guiCursor().connect(work_areas_[id].get());

	return *work_areas_[id].get();
}


void GuiImplementation::destroyWorkArea(int id)
{
	work_areas_.erase(id);
}

} // namespace frontend
} // namespace lyx
