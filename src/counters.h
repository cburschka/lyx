// -*- C++ -*-

#ifndef COUNTERS_H
#define COUTNERS_H

#include <map>
#include <sigc++/signal_system.h>
#include "LString.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
using SigC::Signal0;
#endif


///
class Counter : public Object {
public:
	///
	Counter();
	///
	void set(int v);
	///
	void addto(int v);
	///
	int value() const;
	///
	void step();
	///
	void reset();
	///
	Signal0<void> onstep;
private:
	///
	int value_;
};


/** This is a class of (La)TeX type counters. The counters is in a text
    Style and can be reset by signals emitted from a single counter.
*/
class Counters {
public:
	///
	~Counters();
	///
	void newCounter(string const & newc);
	///
	void newCounter(string const & newc, string const & oldc);
	///
	void set(string const & ctr, int val);
	///
	void addto(string const & ctr, int val);
	///
	int value(string const & ctr) const;
	///
	void step(string const & ctr);
	// string refstep(string const & cou);
private:
	///
	typedef std::map<string, Counter*> CounterList;
	///
	CounterList counterList;
};

#endif
