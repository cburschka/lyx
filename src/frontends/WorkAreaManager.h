// -*- C++ -*-
/**
 * \file WorkAreaManager.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORKAREA_MANAGER_H
#define WORKAREA_MANAGER_H

#include <vector>

namespace lyx {
namespace frontend {

class WorkArea;

class WorkAreaManager
{
public:
	WorkAreaManager() {}

	///
	void registerWorkArea(WorkArea * wa);

	///
	void changed();

	///
	void closing();

private:
	std::vector<WorkArea *> work_areas_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
