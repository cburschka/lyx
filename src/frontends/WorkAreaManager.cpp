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

namespace lyx {
namespace frontend {

void WorkAreaManager::registerWorkArea(WorkArea * wa)
{
	work_areas_.push_back(wa);
}

void WorkAreaManager::changed()
{
	for (size_t i = 0; i != work_areas_.size(); ++i)
		work_areas_[i]->redraw();
}

void WorkAreaManager::closing()
{
	for (size_t i = 0; i != work_areas_.size(); ++i)
		work_areas_[i]->close();
}

} // namespace frontend
} // namespace lyx

