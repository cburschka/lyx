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

#include "WorkAreaManager.h"

#include "WorkArea.h"


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


void WorkAreaManager::redrawAll(bool update_metrics)
{
	iterator it = work_areas_.begin();
	iterator const en = work_areas_.end();
	for (; it != en; ++it)
		(*it)->redraw(update_metrics);
}


void WorkAreaManager::closeAll()
{
	while (!work_areas_.empty())
		// WorkArea is de-registering itself.
		(*work_areas_.begin())->close();
}


void WorkAreaManager::updateTitles()
{
	iterator it = work_areas_.begin();
	iterator const en = work_areas_.end();
	for (; it != en; ++it)
		(*it)->updateWindowTitle();
}

} // namespace frontend
} // namespace lyx

