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
	for (list<WorkArea *>::iterator it = work_areas_.begin();
		it != work_areas_.end(); ) {
		(*it)->close();
		if (work_areas_.empty())
			break;
		++it;
	}
}

} // namespace frontend
} // namespace lyx

