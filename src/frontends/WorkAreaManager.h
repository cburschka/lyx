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

#include <list>

namespace lyx {

class Buffer;

namespace frontend {

class WorkArea;

/// \c WorkArea Manager.
/**
  * This is a helper class designed to avoid signal/slot connections
  * between a \c Buffer and the potentially multiple \c WorkArea(s)
  * used to visualize this Buffer contents.
  */
class WorkAreaManager
{
public:
	///
	WorkAreaManager() {}
	///
	void add(WorkArea * wa);
	///
	void remove(WorkArea * wa);
	///
	void redrawAll(bool update_metrics);
	///
	void closeAll();
	/// Update window titles of all users and the external modifications
	/// warning.
	void updateTitles();
	/// If there is no work area, create a new one in the current view using the
	/// buffer buf. Returns false if not possible.
	bool unhide(Buffer * buf) const;

private:
	typedef std::list<WorkArea *>::iterator iterator;
	///
	std::list<WorkArea *> work_areas_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
