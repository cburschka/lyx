/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "counters.h"
#include "debug.h"

using std::endl;


Counter::Counter()
{
	reset();
}


void Counter::set(int v)
{
	value_ = v;
}


void Counter::addto(int v)
{
	value_ += v;
}


int Counter::value() const
{
	return value_;
}


void Counter::step()
{
	++value_;
	onstep.emit();
}


void Counter::reset()
{
	value_ = 0;
}


Counters::~Counters() 
{
	// We need this since we store the Counter's as pointers in
	// the counterList.
	for (CounterList::iterator it = counterList.begin();
	     it != counterList.end();
	     ++it)
		delete (*it).second;
}


void Counters::newCounter(string const & newc)
{
	// First check if newc already exist
	CounterList::iterator cit = counterList.find(newc);
	// if alrady exist give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exist." << endl;
		return;
	}
	counterList[newc] = new Counter;
}


void Counters::newCounter(string const & newc, string const & oldc)
{
	// First check if newc already exist
	CounterList::iterator cit = counterList.find(newc);
	// if already existant give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exist." << endl;
		return;
	}
	// then check if oldc exist
	CounterList::iterator it = counterList.find(oldc);
	// if not give warning and return
	if (it == counterList.end()) {
		lyxerr << "The old counter does not exist." << endl;
		return;
	}

	Counter * tmp = new Counter;
	(*it).second->onstep.connect(SigC::slot(tmp,
					 &Counter::reset));
	counterList[newc] = tmp;
}


void Counters::set(string const & ctr, int val) 
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "Counter does not exist." << endl;
		return;
	}
	(*it).second->set(val);
}


void Counters::addto(string const & ctr, int val)
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "Counter does not exist." << endl;
		return;
	}
	(*it).second->addto(val);
}


int Counters::value(string const & ctr) const 
{
	CounterList::const_iterator cit = counterList.find(ctr);
	if (cit == counterList.end()) {
		lyxerr << "Counter does not exist." << endl;
		return 0;
	}
	return (*cit).second->value();
}


void Counters::step(string const & ctr)
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "Counter does not exist." << endl;
		return;
	}
	(*it).second->step();
}
