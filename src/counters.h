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


/// This represents a single counter.
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
	/// Returns the master counter of this counter
	string master() const;
	/// sets the master counter for this counter
	void setMaster(string const & m);
private:
	///
	int value_;
	/// contains master counter name; master counter is the counter
	/// that, if stepped (incremented) zeroes this counter. E.g.
	/// "subparagraph"'s master is "paragraph".
	string master_;
};


/// This is a class of (La)TeX type counters.
/// Every instantiation is an array of counters of type Counter.
class Counters {
public:
	///
	Counters();
	///
	//~Counters();
	/// Add a new counter to array.
	void newCounter(string const & newc);
	/// Add new counter having oldc as its master.
	void newCounter(string const & newc, string const & oldc);
	///
	void set(string const & ctr, int val);
	///
	void addto(string const & ctr, int val);
	///
	int value(string const & ctr) const;
	/// Step (increment by one) counter named by arg, and
	/// zeroes slave counter(s) for which it is the master.
	/// NOTE sub-slaves not zeroed! That happens at slave's
	/// first step 0->1. Seems to be sufficient.
	void step(string const & ctr);
	/// Reset all counters.
	void reset();
	/// Reset counters matched by match string.
	void reset(string const & match);
	/// Copy counters whose name matches match from the &from to
	/// the &to array of counters. Empty string matches all.
	void copy(Counters & from, Counters & to, string const & match = string());
	/// A numeric label's single item, like .1 for subsection number in
	/// the 2.1.4 subsubsection number label. "first" indicates if this
	/// is the first item to be displayed, usually chapter or section.
	string labelItem(string const & ctr,
			string const & labeltype,
			string const & langtype = "latin",
			bool first = false);
	/// A complete numeric label, like 2.1.4 for a subsubsection.
	/// "head" indicates sequence number of first item to be
	/// displayed, e.g. 0 for chapter, 1 for section.
	string numberLabel(string const & ctr,
			string const & labeltype,
			string const & langtype = "latin",
			int head = 0);
private:
	/// Maps counter (layout) names to actual counters.
	typedef std::map<string, Counter> CounterList;
	/// Instantiate.
	CounterList counterList;

};

#endif
