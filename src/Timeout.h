// -*- C++ -*-
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <sigc++/signal_system.h>

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
		ONETIME,
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
