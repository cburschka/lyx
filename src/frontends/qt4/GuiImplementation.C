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

	QObject::connect(views_[id], SIGNAL(destroyed(QObject *)),
		this, SLOT(cleanupViews(QObject *)));

	return id;
}


LyXView& GuiImplementation::view(int id)
{
	BOOST_ASSERT(views_.find(id) != views_.end());

	return *views_[id];
}


void GuiImplementation::cleanupViews(QObject * qobj)
{
	GuiView * view = static_cast<GuiView *>(qobj);
	std::map<int, GuiView *>::iterator I;

	for (I = views_.begin(); I != views_.end(); ++I) {
		if (I->second == view) {
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

	view->setCentralWidget(work_areas_[id]);

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
