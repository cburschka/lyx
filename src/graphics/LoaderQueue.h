// -*- C++ -*-
/**
 *  \file LoaderQueue.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Alfredo Braunstein
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
#include "GraphicsCacheItem.h"

#include "frontends/Timeout.h"

#include <set>
#include <queue>

namespace grfx {

class LoaderQueue {
public:
	//use this to request a loading
	void touch(Cache::ItemPtr const & item);
	//query if the clock is ticking
	bool running() const;
	//get the and only instance of the class
	static LoaderQueue & get();
private:
	//this class is a singleton class... use LoaderQueue::get() instead
	LoaderQueue();
	//in-progress loading queue (elements are unique here) 
	std::list<Cache::ItemPtr> cache_queue_;
	//makes faster the insertion of new elements
	std::set<Cache::ItemPtr> cache_set_;
	//newly touched element go here, loadNext move them to cache_queue_
	std::queue<Cache::ItemPtr> bucket_;
	//
	Timeout timer;
	//
	bool running_;
	//moves bucket_ to cache_queue_
	void emptyBucket();
	//adds or reprioritizes one element in cache_queue_
	void addToQueue(Cache::ItemPtr const & item);
	//this is the 'threaded' method, that does the loading in background
	void loadNext();
	//
	void startLoader();
	//
	void stopLoader();
};

} // namespace grfx

#endif // LOADERQUEUE_H
