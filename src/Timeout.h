// -*- C++ -*-
#ifndef TIMEOUT_H
#define TIMEOUT_H

/** This class executes the callback when the timeout expires.
    This class currently uses a regular callback, later it will use
    signals and slots to provide the same.
*/
class Timeout {
public:
	///
	typedef void (*TimeoutCallback)(void *);
	
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
	void callback(TimeoutCallback cb, void * data);
	///
	void callback();
	///
	void setType(Type t);
	///
	void setTimeout(int msec);
private:
	///
	Type type;
	///
	int timeout;
	///
	int timeout_id;
	///
	TimeoutCallback callback_;
	///
	void * data_;
};

#endif
