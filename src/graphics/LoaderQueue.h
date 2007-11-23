// -*- C++ -*-
/**
 * \file LoaderQueue.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 *
 * This implements a threaded service queue which loads images on background.
 * In order to request an image loading you call touch() with the pointer to
 * the cached image. Then it will try to satisfy the request as soon as
 * posible (that's it: after finishing an eventual loading on progress)
 * touch() returns inmediately, in order not tu disrupt the flow of the main
 * thread.
 * The service thread is the method loadNext(). It's actually not a thread,
 * but implemented with a timer that comes back every x msec.
 */

#ifndef LOADERQUEUE_H
#define LOADERQUEUE_H

#include "GraphicsCache.h"
#include "support/Timeout.h"

#include <set>
#include <queue>

namespace lyx {
namespace graphics {

class LoaderQueue {
public:
	/// Use this to request that the item is loaded.
	void touch(Cache::ItemPtr const & item);
	/// Query whether the clock is ticking.
	bool running() const;
	///get the and only instance of the class
	static LoaderQueue & get();
private:
	/// This class is a singleton class... use LoaderQueue::get() instead
	LoaderQueue();
	/// The in-progress loading queue (elements are unique here).
	std::list<Cache::ItemPtr> cache_queue_;
	/// Used to make the insertion of new elements faster.
	std::set<Cache::ItemPtr> cache_set_;
	/// Newly touched elements go here. loadNext moves them to cache_queue_
	std::queue<Cache::ItemPtr> bucket_;
	///
	Timeout timer;
	///
	bool running_;

	/** This is the 'threaded' method, that does the loading in the
	 *  background.
	 */
	void loadNext();
	///
	void startLoader();
	///
	void stopLoader();
};

} // namespace graphics
} // namespace lyx

#endif // LOADERQUEUE_H
