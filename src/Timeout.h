// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <config.h>

#ifdef __GNUG__
#pragma interface
#endif

#include <sigc++/signal_system.h>
/* ugly hack to prevent Qt's '#define emit ...' from 
 * screwing us up below - jbl 2000/8/10 
 */
#ifdef KDEGUI
#undef emit
#endif


#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif

/** This class executes the callback when the timeout expires.
    This class currently uses a regular callback, later it will use
    signals and slots to provide the same.
*/
class Timeout {
public:
	///
	enum Type {
		///
		ONETIME,
		///
		CONTINOUS
	};
	///
	Timeout();
	///
	Timeout(int msec, Type = ONETIME);
	///
	~Timeout();
	///
	void start();
	///
	void stop();
	///
	void restart();
	///
	Signal0<void> timeout;
	///
	void emit();
	///
	void setType(Type t);
	///
	void setTimeout(int msec);
private:
	///
	Type type;
	///
	int timeout_ms;
	///
	int timeout_id;
};

#endif
