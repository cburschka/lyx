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
	Counter(docstring const & mc, docstring const & ls, 
		docstring const & lsa);
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
	docstring const & master() const;
	/// Returns a LaTeX-like string to format the counter, similar
	/// to LaTeX' \c \thesubsection.
	docstring const & labelString() const;
	/// Returns a LaTeX-like string to format the counter in
	/// appendix, similar to LaTeX' \c \thesubsection.
	docstring const & labelStringAppendix() const;
private:
	///
	int value_;
	/// contains master counter name; master counter is the counter
	/// that, if stepped (incremented) zeroes this counter. E.g.
	/// "subsection"'s master is "section".
	docstring master_;
	// Contains a LaTeX-like string to format the counter, similar
	// to LaTeX' \c \thesubsection.
	docstring labelstring_;
	// The same as labelstring_, but in appendices.
	docstring labelstringappendix_;
};


/// This is a class of (La)TeX type counters.
/// Every instantiation is an array of counters of type Counter.
class Counters {
public:
	/// Add a new counter to array.
	void newCounter(docstring const & newc);
	/// Add new counter having oldc as its master and ls as its label.
	void newCounter(docstring const & newc,
			docstring const & masterc,
			docstring const & ls,
			docstring const & lsa);
	///
	bool hasCounter(docstring const & c) const;
	///
	void set(docstring const & ctr, int val);
	///
	void addto(docstring const & ctr, int val);
	///
	int value(docstring const & ctr) const;
	/// Step (increment by one) counter named by arg, and
	/// zeroes slave counter(s) for which it is the master.
	/// NOTE sub-slaves not zeroed! That happens at slave's
	/// first step 0->1. Seems to be sufficient.
	void step(docstring const & ctr);
	/// Reset all counters.
	void reset();
	/// Reset counters matched by match string.
	void reset(docstring const & match);
	/// Copy counters whose name matches match from the &from to
	/// the &to array of counters. Empty string matches all.
	void copy(Counters & from, Counters & to,
		  docstring const & match = docstring());
	/// returns the string representation of the counter.
	docstring theCounter(docstring const & c);
	/// A complete expanded label, like 2.1.4 for a subsubsection
	/// according to the given format
	docstring counterLabel(docstring const & format);
	///
	bool appendix() const { return appendix_; };
	///
	void appendix(bool a) { appendix_ = a; };
	///
	std::string const & current_float() const { return current_float_; }
	///
	void current_float(std::string const & f) { current_float_ = f; }
private:
	/// A counter label's single item, 1 for subsection number in
	/// the 2.1.4 subsubsection number label.
	docstring labelItem(docstring const & ctr,
			    docstring const & numbertype);
	/// Maps counter (layout) names to actual counters.
	typedef std::map<docstring, Counter> CounterList;
	/// Instantiate.
	CounterList counterList;
	///
	bool appendix_;
	///
	std::string current_float_;
};


} // namespace lyx

#endif
