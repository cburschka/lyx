/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

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
		to->emit();
	}
}


Timeout::Timeout()
	: type(ONETIME), timeout_ms(0), timeout_id(-1)
{}


Timeout::Timeout(unsigned int msec, Type t)
	: type(t), timeout_ms(msec), timeout_id(-1)
{}


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
	timeout_id = fl_add_timeout(timeout_ms,
				    C_intern_timeout_cb, this);
}
	

void Timeout::stop()
{
	if (timeout_id != -1) {
		fl_remove_timeout(timeout_id);
		timeout_id = -1;
	}
}


void Timeout::emit()
{
	timeout_id = -1;
	timeout.emit();
	if (type == CONTINOUS)
		start();
}
	

Timeout & Timeout::setType(Type t)
{
	type = t;
	return *this;
}


Timeout & Timeout::setTimeout(unsigned int msec)
{
	timeout_ms = msec;
	return *this;
}
