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


void WorkAreaManager::redrawAll()
{
	for (iterator it = work_areas_.begin(); it != work_areas_.end(); ++it)
		(*it)->redraw();
}


void WorkAreaManager::closeAll()
{
	while (!work_areas_.empty())
		// WorkArea is de-registering itself.
		(*work_areas_.begin())->close();
}


void WorkAreaManager::setReadOnly(bool on)
{
	for (iterator it = work_areas_.begin(); it != work_areas_.end(); ++it)
		(*it)->setReadOnly(on);
}


void WorkAreaManager::updateTitles()
{
	for (iterator it = work_areas_.begin(); it != work_areas_.end(); ++it)
		(*it)->updateWindowTitle();
}

} // namespace frontend
} // namespace lyx

