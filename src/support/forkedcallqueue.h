// -*- C++ -*-
/**
 * \file forkedcallqueue.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein (based on an idea from Angus Leeming)
 *
 * Full author contact details are available in file CREDITS.
 *
 * This class implements a queue of forked processes. In order not to
 * hose the system with multiple processes running simultaneously, you can
 * request the addition of your process to this queue and it will be
 * executed when its turn comes.
 *
 */

#ifndef FORKEDCALLQUEUE_H
#define FORKEDCALLQUEUE_H

#include "forkedcall.h"

#include <queue>
#include <utility>
#include "support/std_string.h"

namespace lyx {
namespace support {

class ForkedCallQueue {
public:
	/// A process in the queue
	typedef std::pair<string, Forkedcall::SignalTypePtr> Process;
        /** Add a process to the queue. Processes are forked sequentially
	 *  only one is running at a time.
         *  Connect to the returned signal and you'll be informed when
         *  the process has ended.
         */
	Forkedcall::SignalTypePtr add(string const & process);
	/// Query whether the queue is running a forked process now.
	bool running() const;
	/// Get the and only instance of the class
	static ForkedCallQueue & get();

private:

	/** this class is a singleton class... use
	 *  ForkedCallQueue::get() instead
	 */
	ForkedCallQueue();
	/// in-progress queue
	std::queue<Process> callQueue_;
	///
	bool running_;
	///
	void callNext();
	///
	void startCaller();
	///
	void stopCaller();
	///
	void callback(pid_t, int);
};

} // namespace support
} // namespace lyx

#endif // FORKEDCALLQUEUE_H
