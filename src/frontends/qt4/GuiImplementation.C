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
#include "bufferlist.h"
#include "funcrequest.h"
#include "lyxfunc.h"

using boost::shared_ptr;

namespace lyx {
namespace frontend {


GuiImplementation::GuiImplementation(): max_view_id_(0), max_wa_id_(0)
{
}


int GuiImplementation::newView()
{
	size_t const id = max_view_id_;
	++max_view_id_;

	views_[id] = new GuiView(id);
	view_ids_.push_back(id);

	return id;
}


LyXView& GuiImplementation::view(int id)
{
	BOOST_ASSERT(views_.find(id) != views_.end());

	return *views_[id];
}


bool GuiImplementation::closeAll()
{
	if (!theBufferList().quitWriteAll())
		return false;

	// In order to know if it is the last opened window,
	// GuiView::closeEvent() check for (view_ids_.size() == 1)
	// We deny this check by setting the vector size to zero.
	// But we still need the vector, hence the temporary copy.
	std::vector<int> view_ids_tmp = view_ids_;
	view_ids_.clear();

	for (size_t i = 0; i < view_ids_tmp.size(); ++i) {
		// LFUN_LYX_QUIT has already been triggered so we need
		// to disable the lastWindowClosed() signal before closing
		// the last window.
		views_[view_ids_tmp[i]]->setAttribute(Qt::WA_QuitOnClose, false);
		views_[view_ids_tmp[i]]->close();
		// The view_ids_ vector is reconstructed in the closeEvent; so
		// let's clear that out again!
		view_ids_.clear();
	}

	views_.clear();
	view_ids_.clear();
	work_areas_.clear();

	return true;
}


void GuiImplementation::unregisterView(GuiView * view)
{
	std::map<int, GuiView *>::iterator I;

	for (I = views_.begin(); I != views_.end(); ++I) {
		if (I->second == view) {
			std::vector<int> const & wa_ids = view->workAreaIds();
			for (size_t i = 0; i < wa_ids.size(); ++i)
				work_areas_.erase(wa_ids[i]);

			views_.erase(I->first);
			break;
		}
	}

	buildViewIds();

	if (views_.empty()) {
		theLyXFunc().setLyXView(0);
//		dispatch(FuncRequest(LFUN_LYX_QUIT));
		return;
	}

	theLyXFunc().setLyXView(views_.begin()->second);
}


void GuiImplementation::buildViewIds()
{
	view_ids_.clear();
	std::map<int, GuiView *>::const_iterator I;
	for (I = views_.begin(); I != views_.end(); ++I)
		view_ids_.push_back(I->first);
}


int GuiImplementation::newWorkArea(unsigned int w, unsigned int h, int view_id)
{
	size_t const id = max_wa_id_;
	++max_wa_id_;

	GuiView * view = views_[view_id];

	work_areas_[id] = new GuiWorkArea(w, h, id, *view);

	// FIXME BufferView creation should be independant of WorkArea creation
	buffer_views_[id].reset(new BufferView);
	work_areas_[id]->setBufferView(buffer_views_[id].get());

	view->setWorkArea(work_areas_[id]);
	view->initTab(work_areas_[id]);
	work_areas_[id]->setFocus();

	return id;
}


WorkArea& GuiImplementation::workArea(int id)
{
	BOOST_ASSERT(work_areas_.find(id) != work_areas_.end());

	return *work_areas_[id];
}


} // namespace frontend
} // namespace lyx

#include "GuiImplementation_moc.cpp"
