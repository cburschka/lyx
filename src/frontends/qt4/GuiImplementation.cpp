// -*- C++ -*-
/**
 * \file GuiImplementation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiImplementation.h"
#include "GuiView.h"

#include <boost/assert.hpp>

#include <QApplication>

using std::map;
using std::vector;


namespace lyx {
namespace frontend {


static void updateIds(map<int, GuiView *> const & stdmap, vector<int> & ids)
{
	ids.clear();
	map<int, GuiView *>::const_iterator it;
	for (it = stdmap.begin(); it != stdmap.end(); ++it)
		ids.push_back(it->first);
}


GuiImplementation::GuiImplementation()
{
	view_ids_.clear();
}


int GuiImplementation::createRegisteredView()
{
	updateIds(views_, view_ids_);
	int id = 0;
	while (views_.find(id) != views_.end())
		id++;
	views_[id] = new GuiView(id);
	updateIds(views_, view_ids_);
	return id;
}


bool GuiImplementation::unregisterView(int id)
{
	updateIds(views_, view_ids_);
	BOOST_ASSERT(views_.find(id) != views_.end());
	BOOST_ASSERT(views_[id]);

	map<int, GuiView *>::iterator it;
	for (it = views_.begin(); it != views_.end(); ++it) {
		if (it->first == id) {
			views_.erase(id);
			break;
		}
	}
	updateIds(views_, view_ids_);
	return true;
}


bool GuiImplementation::closeAllViews()
{
	updateIds(views_, view_ids_);
	if (views_.empty()) {
		// quit in CloseEvent will not be triggert
		qApp->quit();
		return true;
	}

	map<int, GuiView*> const cmap = views_;
	map<int, GuiView*>::const_iterator it;
	for (it = cmap.begin(); it != cmap.end(); ++it) {
		// TODO: return false when close event was ignored
		//       e.g. quitWriteAll()->'Cancel'
		//       maybe we need something like 'bool closeView()'
		it->second->close();
		// unregisterd by the CloseEvent
	}

	views_.clear();
	view_ids_.clear();
	return true;
}


LyXView & GuiImplementation::view(int id) const
{
	BOOST_ASSERT(views_.find(id) != views_.end());
	return *views_.find(id)->second;
}


} // namespace frontend
} // namespace lyx

#include "GuiImplementation_moc.cpp"
