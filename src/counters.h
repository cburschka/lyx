// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *
 * ====================================================== */


#ifndef COUNTERS_H
#define COUNTERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include <sigc++/signal_system.h>
#include "LString.h"

///
class Counter : public SigC::Object {
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
	SigC::Signal0<void> onstep;
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
