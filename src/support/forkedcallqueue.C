/**
 * \file forkedcallqueue.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein (based on an idea from Angus Leeming)
 * 
 * Full author contact details are available in file CREDITS
 */

#include "forkedcallqueue.h"

#include "debug.h"

#include <boost/bind.hpp>
#include <boost/signals/signal2.hpp>

using std::endl;
using std::queue;

ForkedCallQueue & ForkedCallQueue::get()
{
	static ForkedCallQueue singleton;
	return singleton;
}


Forkedcall::SignalTypePtr ForkedCallQueue::add(string const & process)
{
	Forkedcall::SignalTypePtr ptr;
	ptr.reset(new Forkedcall::SignalType);
	callQueue_.push(Process(process, ptr));
	if (!running_) {
		startCaller();
	}
	return ptr;
}


void ForkedCallQueue::callNext()
{
	if (callQueue_.empty())
		return;
	Process pro = callQueue_.front();
	callQueue_.pop();
	// Bind our chain caller
	pro.second->connect(boost::bind(&ForkedCallQueue::callback, 
					 this, _1, _2));
	Forkedcall call;
	// If we fail to fork the process, then emit the signal
	// to tell the outside world that it failed.
	if (call.startscript(pro.first, pro.second) > 0) {
		pro.second->operator()(0,1);
	}
}


void ForkedCallQueue::callback(pid_t, int)
{
	if(callQueue_.empty()) {
		stopCaller();
	} else {
		callNext();
	}
}

ForkedCallQueue::ForkedCallQueue() : running_(false)
{}

	
void ForkedCallQueue::startCaller()
{
	lyxerr[Debug::GRAPHICS] << "ForkedCallQueue: waking up" << endl;
	running_ = true ;
	callNext();
}


void ForkedCallQueue::stopCaller()
{
	running_ = false ;
	lyxerr[Debug::GRAPHICS] << "ForkedCallQueue: I'm going to sleep" 
				<< endl;
}


bool ForkedCallQueue::running() const
{
	return running_ ;
}
