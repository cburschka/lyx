// -*- C++ -*-
/**
 * \file Counters.h
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

#include "support/docstring.h"

#include <map>


namespace lyx {

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
	lyx::docstring const & master() const;
	/// sets the master counter for this counter
	void setMaster(lyx::docstring const & m);
private:
	///
	int value_;
	/// contains master counter name; master counter is the counter
	/// that, if stepped (incremented) zeroes this counter. E.g.
	/// "subparagraph"'s master is "paragraph".
	lyx::docstring master_;
};


/// This is a class of (La)TeX type counters.
/// Every instantiation is an array of counters of type Counter.
class Counters {
public:
	/// Add a new counter to array.
	void newCounter(lyx::docstring const & newc);
	/// Add new counter having oldc as its master.
	void newCounter(lyx::docstring const & newc,
			lyx::docstring const & oldc);
	///
	void set(lyx::docstring const & ctr, int val);
	///
	void addto(lyx::docstring const & ctr, int val);
	///
	int value(lyx::docstring const & ctr) const;
	/// Step (increment by one) counter named by arg, and
	/// zeroes slave counter(s) for which it is the master.
	/// NOTE sub-slaves not zeroed! That happens at slave's
	/// first step 0->1. Seems to be sufficient.
	void step(lyx::docstring const & ctr);
	/// Reset all counters.
	void reset();
	/// Reset counters matched by match string.
	void reset(lyx::docstring const & match);
	/// Copy counters whose name matches match from the &from to
	/// the &to array of counters. Empty string matches all.
	void copy(Counters & from, Counters & to,
		  lyx::docstring const & match = lyx::docstring());
	/// A complete expanded label, like 2.1.4 for a subsubsection
	/// according to the given format
	lyx::docstring counterLabel(lyx::docstring const & format);
	///
	bool appendix() const { return appendix_; };
	///
	void appendix(bool a) { appendix_ = a; };
private:
	/// A counter label's single item, 1 for subsection number in
	/// the 2.1.4 subsubsection number label.
	lyx::docstring labelItem(lyx::docstring const & ctr,
				 lyx::docstring const & numbertype);
	/// Maps counter (layout) names to actual counters.
	typedef std::map<lyx::docstring, Counter> CounterList;
	/// Instantiate.
	CounterList counterList;
	///
	bool appendix_;

};


} // namespace lyx

#endif
