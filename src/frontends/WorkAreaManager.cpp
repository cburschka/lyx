// -*- C++ -*-
/**
 * \file WorkAreaManager.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "WorkArea.h"

#include "WorkAreaManager.h"

using std::list;

namespace lyx {

extern bool quitting;

namespace frontend {

void WorkAreaManager::add(WorkArea * wa)
{
	work_areas_.push_back(wa);
}


void WorkAreaManager::remove(WorkArea * wa)
{
	work_areas_.remove(wa);
}


void WorkAreaManager::redrawAll()
{
	for (list<WorkArea *>::iterator it = work_areas_.begin();
		it != work_areas_.end(); ) {
		(*it)->redraw();
		++it;
	}
}


void WorkAreaManager::closeAll()
{
	if (quitting)
		return;

	while (!work_areas_.empty())
		// WorkArea is de-registering itself.
		(*work_areas_.begin())->close();
}

} // namespace frontend
} // namespace lyx

