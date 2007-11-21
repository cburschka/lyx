/**
 * \file LoaderQueue.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LoaderQueue.h"
#include "GraphicsCacheItem.h"

#include "debug.h"

#include <boost/bind.hpp>

using std::endl;
using std::list;

namespace lyx {
namespace graphics {

static int s_numimages_ = 5;
static int s_millisecs_ = 500;


LoaderQueue & LoaderQueue::get()
{
	static LoaderQueue singleton;
	return singleton;
}


void LoaderQueue::loadNext()
{
	LYXERR(Debug::GRAPHICS, "LoaderQueue: "
		<< cache_queue_.size() << " items in the queue");
	int counter = s_numimages_;
	while (cache_queue_.size() && counter--) {
		Cache::ItemPtr ptr = cache_queue_.front();
		cache_set_.erase(ptr);
		cache_queue_.pop_front();
		if (ptr->status() == WaitingToLoad)
			ptr->startLoading();
	}
	if (cache_queue_.size()) {
		startLoader();
	} else {
		stopLoader();
	}
}


void LoaderQueue::setPriority(int numimages , int millisecs)
{
	s_numimages_ = numimages;
	s_millisecs_ = millisecs;
	LYXERR(Debug::GRAPHICS, "LoaderQueue:  priority set to "
				<< s_numimages_ << " images at a time, "
				<< s_millisecs_ << " milliseconds between calls");
}


LoaderQueue::LoaderQueue() : timer(s_millisecs_, Timeout::ONETIME),
			     running_(false)
{
	timer.timeout.connect(boost::bind(&LoaderQueue::loadNext, this));
}


void LoaderQueue::startLoader()
{
	LYXERR(Debug::GRAPHICS, "LoaderQueue: waking up");
	running_ = true ;
	timer.setTimeout(s_millisecs_);
	timer.start();
}


void LoaderQueue::stopLoader()
{
	timer.stop();
	running_ = false ;
	LYXERR(Debug::GRAPHICS, "LoaderQueue: I'm going to sleep");
}


bool LoaderQueue::running() const
{
	return running_ ;
}


void LoaderQueue::touch(Cache::ItemPtr const & item)
{
	if (! cache_set_.insert(item).second) {
		list<Cache::ItemPtr>::iterator
			it = cache_queue_.begin();
		list<Cache::ItemPtr>::iterator
			end = cache_queue_.end();

		it = std::find(it, end, item);
		if (it != end)
			cache_queue_.erase(it);
	}
	cache_queue_.push_front(item);
	if (!running_)
		startLoader();
}


} // namespace graphics
} // namespace lyx
