#include <config.h>

#include FORMS_H_LOCATION

#include "Timeout.h"
#include "debug.h"

using std::endl;

extern "C" {
	static
	void C_intern_timeout_cb(int, void * data) 
	{
		Timeout * to = static_cast<Timeout*>(data);
		to->callback();
	}
}


Timeout::Timeout()
	: type(ONETIME), timeout(0), timeout_id(-1),
	 callback_(0), data_(0) {}


Timeout::Timeout(int msec, Type t = ONETIME)
	: type(t), timeout(msec), timeout_id(-1),
	  callback_(0), data_(0) {}


Timeout::~Timeout() 
{
	stop();
}


void Timeout::restart()
{
	stop();
	start();
}


void Timeout::start()
{
	if (timeout_id != -1)
		lyxerr << "Timeout::start: already running!" << endl;
	timeout_id = fl_add_timeout(timeout,
				    C_intern_timeout_cb, this);
}
	

void Timeout::stop()
{
	if (timeout_id != -1) {
		fl_remove_timeout(timeout_id);
		timeout_id = -1;
	}
}


void Timeout::callback(TimeoutCallback cb, void * data)
{
	callback_ = cb;
	data_ = data;
}


void Timeout::callback()
{
	timeout_id = -1;
	if (callback_)
		callback_(data_);
	if (type == CONTINOUS)
		start();
}
	

void Timeout::setType(Type t)
{
	type = t;
}


void Timeout::setTimeout(int msec)
{
	timeout = msec;
}
