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

#include "LString.h"
#include <map>
#include <vector>

///
class Counter {
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
	string master() const;
	///
	void setMaster(string const & m);
	///

private:
	int value_;
	///
	string master_;
};


/** This is a class of (La)TeX type counters. The counters is in a text
    Style and can be reset by signals emitted from a single counter.
*/
class Counters {
public:
	///
	Counters();
	///	
	//~Counters();
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
	///
	void reset(string const & match = "");
	///
	void copy(Counters & from, Counters & to, string const & match = "");
	///
	string labelItem(string const & ctr,
			string const & labeltype, 
			string const & langtype = "latin",
			bool first = false);
	///
	string numberLabel(string const & ctr,
			string const & labeltype, 
			string const & langtype = "latin",
			int head = 0);
	///
	std::vector<string> enums, sects;
	
private:
	///
	typedef std::map<string, Counter> CounterList;
	///
	CounterList counterList;

};

#endif
