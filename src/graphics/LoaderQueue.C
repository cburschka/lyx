/**
 * \file LoaderQueue.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include "LoaderQueue.h"

#include "debug.h"

#include <boost/bind.hpp>

using std::endl;
using std::list;


namespace grfx {


LoaderQueue & LoaderQueue::get()
{
	static LoaderQueue singleton;
	return singleton;
}


void LoaderQueue::loadNext() 
{
	emptyBucket();
	lyxerr[Debug::GRAPHICS] << "LoaderQueue: "
				<< cache_queue_.size()
				<< " items in the queue" << endl; 
	int counter = 10;
	while (cache_queue_.size() && counter--) {
		if(cache_queue_.front()->status() == WaitingToLoad)
			cache_queue_.front()->startLoading();
		cache_set_.erase(cache_queue_.front());
		cache_queue_.pop_front();
	}
	if (cache_queue_.size() || bucket_.size()) {
		startLoader();
	} else {
		stopLoader();
	}
}


LoaderQueue::LoaderQueue() : timer(100, Timeout::ONETIME), 
			     running_(false)
{
	timer.timeout.connect(boost::bind(&LoaderQueue::loadNext, this));
}

	
void LoaderQueue::emptyBucket()
{
	lyxerr[Debug::GRAPHICS] << "LoaderQueue: emptying bucket" 
				<< endl;
	while (! bucket_.empty()) {
		addToQueue(bucket_.front());
		bucket_.pop();
	}
}


void LoaderQueue::startLoader()
{
	lyxerr[Debug::GRAPHICS] << "LoaderQueue: waking up" << endl;
	running_ = true ;
	timer.start();
}


void LoaderQueue::stopLoader()
{
	timer.stop();
	running_ = false ;
	lyxerr[Debug::GRAPHICS] << "LoaderQueue: I'm going to sleep" << endl;
}


bool LoaderQueue::running() const
{
	return running_ ;
}


void LoaderQueue::touch(Cache::ItemPtr const & item)
{
	if (! running_)
		startLoader();
	bucket_.push(item);
}


void LoaderQueue::addToQueue(Cache::ItemPtr const & item)
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
}


} // namespace grfx
