// -*- C++ -*-
/**
 * \file counters.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COUNTERS_H
#define COUNTERS_H

#include <map>
#include <string>


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
	std::string master() const;
	/// sets the master counter for this counter
	void setMaster(std::string const & m);
private:
	///
	int value_;
	/// contains master counter name; master counter is the counter
	/// that, if stepped (incremented) zeroes this counter. E.g.
	/// "subparagraph"'s master is "paragraph".
	std::string master_;
};


/// This is a class of (La)TeX type counters.
/// Every instantiation is an array of counters of type Counter.
class Counters {
public:
	/// Add a new counter to array.
	void newCounter(std::string const & newc);
	/// Add new counter having oldc as its master.
	void newCounter(std::string const & newc, std::string const & oldc);
	///
	void set(std::string const & ctr, int val);
	///
	void addto(std::string const & ctr, int val);
	///
	int value(std::string const & ctr) const;
	/// Step (increment by one) counter named by arg, and
	/// zeroes slave counter(s) for which it is the master.
	/// NOTE sub-slaves not zeroed! That happens at slave's
	/// first step 0->1. Seems to be sufficient.
	void step(std::string const & ctr);
	/// Reset all counters.
	void reset();
	/// Reset counters matched by match string.
	void reset(std::string const & match);
	/// Copy counters whose name matches match from the &from to
	/// the &to array of counters. Empty string matches all.
	void copy(Counters & from, Counters & to, std::string const & match = std::string());
	/// A complete expanded label, like 2.1.4 for a subsubsection
	/// according to the given format
	std::string counterLabel(std::string const & format);
	/// A complete label, like 1.a for enumerations
	std::string enumLabel(std::string const & ctr, std::string const & langtype = "latin");
private:
	/// A counter label's single item, 1 for subsection number in
	/// the 2.1.4 subsubsection number label.
	std::string labelItem(std::string const & ctr, std::string const & numbertype);
	/// Maps counter (layout) names to actual counters.
	typedef std::map<std::string, Counter> CounterList;
	/// Instantiate.
	CounterList counterList;

};

#endif
