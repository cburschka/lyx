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

#include "OutputEnums.h"

#include "support/docstring.h"

#include <map>
#include <vector>


namespace lyx {

class Layout;
class Lexer;

/// This represents a single counter.
class Counter {
public:
	///
	Counter();
	///
	Counter(docstring const & mc, docstring const & ls, 
		docstring const & lsa);
	/// \return true on success
	bool read(Lexer & lex);
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
	/// Returns the master counter of this counter.
	docstring const & master() const;
	/// Checks if the master counter is cnt, and if so removes
	/// it. This is used when a counter is deleted.
	/// \return whether we removed the master.
	bool checkAndRemoveMaster(docstring const & cnt);
	/// Returns a LaTeX-like string to format the counter. 
	/** This is similar to what one gets in LaTeX when using
	 *  "\the<counter>". The \c in_appendix bool tells whether we
	 *  want the version shown in an appendix.
	 */
	docstring const & labelString(bool in_appendix) const;
	/// Similar, but used for formatted references in XHTML output.
	/// E.g., for a section counter it might be "section \thesection"
	docstring const & prettyFormat() const { return prettyformat_; }
	
	/// Returns a map of LaTeX-like strings to format the counter. 
	/** For each language, the string is similar to what one gets
	 *  in LaTeX when using "\the<counter>". The \c in_appendix
	 *  bool tells whether we want the version shown in an
	 *  appendix. This version does not contain any \\the<counter>
	 *  expression.
	 */
	typedef std::map<std::string, docstring> StringMap;
	StringMap & flatLabelStrings(bool in_appendix) const;
private:
	///
	int value_;
	/// This is actually one less than the initial value, since the
	/// counter is always stepped before being used.
	int initial_value_;
	/// contains master counter name.
	/** The master counter is the counter that, if stepped
	 *  (incremented) zeroes this counter. E.g. "subsection"'s
	 *  master is "section".
	 */
	docstring master_;
	/// Contains a LaTeX-like string to format the counter.
	docstring labelstring_;
	/// The same as labelstring_, but in appendices.
	docstring labelstringappendix_;
	/// Similar, but used for formatted references in XHTML output
	docstring prettyformat_;
	/// Cache of the labelstring with \\the<counter> expressions expanded, 
	/// indexed by language
	mutable StringMap flatlabelstring_;
	/// Cache of the appendix labelstring with \\the<counter> expressions expanded, 
	/// indexed by language
	mutable StringMap flatlabelstringappendix_;
};


/// This is a class of (La)TeX type counters.
/// Every instantiation is an array of counters of type Counter.
class Counters {
public:
	/// NOTE Do not call this in an attempt to clear the counters.
	/// That will wipe out all the information we have about them
	/// from the document class (e.g., which ones are defined).
	/// Instead, call Counters::reset().
	Counters();
	/// Add new counter newc having masterc as its master, 
	/// ls as its label, and lsa as its appendix label.
	void newCounter(docstring const & newc,
			docstring const & masterc,
			docstring const & ls,
			docstring const & lsa);
	/// Checks whether the given counter exists.
	bool hasCounter(docstring const & c) const;
	/// reads the counter name
	/// \param makeNew whether to make a new counter if one 
	///        doesn't already exist
	/// \return true on success
	bool read(Lexer & lex, docstring const & name, bool makenew);
	///
	void set(docstring const & ctr, int val);
	///
	void addto(docstring const & ctr, int val);
	///
	int value(docstring const & ctr) const;
	/// Reset recursively all the counters that are slaves of the one named by \c ctr.
	void resetSlaves(docstring const & ctr);
	/// Increment by one counter named by \c ctr, and zeroes slave
	/// counter(s) for which it is the master.
	/// \param utype determines whether we track the counters.
	void step(docstring const & ctr, UpdateType utype);
	/// Reset all counters, and all the internal data structures
	/// used for keeping track of their values.
	void reset();
	/// Reset counters matched by match string.
	void reset(docstring const & match);
	/// Remove counter \p cnt.
	bool remove(docstring const & cnt);
	/// Copy counters whose name matches match from the &from to
	/// the &to array of counters. Empty string matches all.
	void copy(Counters & from, Counters & to,
		  docstring const & match = docstring());
	/** returns the expanded string representation of counter \c
	 *  c. The \c lang code is used to translate the string.
	 */
	docstring theCounter(docstring const & c,
			     std::string const & lang) const;
	/** Replace in \c format all the LaTeX-like macros that depend
	 * on counters. The \c lang code is used to translate the
	 * string.
	 */
	docstring counterLabel(docstring const & format,
			       std::string const & lang) const;
	/// returns a formatted version of the counter, using the 
	/// format given by Counter::prettyFormat().
	docstring prettyCounter(docstring const & cntr,
			       std::string const & lang) const;
	/// Are we in appendix?
	bool appendix() const { return appendix_; }
	/// Set the state variable indicating whether we are in appendix.
	void appendix(bool a) { appendix_ = a; }
	/// Returns the current enclosing float.
	std::string const & current_float() const { return current_float_; }
	/// Sets the current enclosing float.
	void current_float(std::string const & f) { current_float_ = f; }
	/// Are we in a subfloat?
	bool isSubfloat() const { return subfloat_; }
	/// Set the state variable indicating whether we are in a subfloat.
	void isSubfloat(bool s) { subfloat_ = s; }
	/// Are we in a longtable?
	bool isLongtable() const { return longtable_; }
	/// Set the state variable indicating whether we are in a longtable.
	void isLongtable(bool s) { longtable_ = s; }
	
	/// \name refstepcounter	
	// @{
	/// The currently active counter, so far as references go.
	/// We're trying to track \refstepcounter in LaTeX, more or less.
	/// Note that this may be empty.
	docstring currentCounter() const;
	/// Called during updateBuffer() as we go through various paragraphs,
	/// to track the layouts as we go through.
	void setActiveLayout(Layout const & lay);
	/// Also for updateBuffer().
	/// Call this when entering things like footnotes, where there is now
	/// no "last layout" and we want to restore the "last layout" on exit.
	void clearLastLayout() { layout_stack_.push_back(0); }
	/// Call this when exiting things like footnotes.
	void restoreLastLayout() { layout_stack_.pop_back(); }
	/// 
	void saveLastCounter()
		{ counter_stack_.push_back(counter_stack_.back()); }
	/// 
	void restoreLastCounter() { counter_stack_.pop_back(); }
	// @}
private:
	/** expands recursively any \\the<counter> macro in the
	 *  labelstring of \c counter.  The \c lang code is used to
	 *  translate the string.
	 */
	docstring flattenLabelString(docstring const & counter, bool in_appendix,
				     std::string const &lang,
				     std::vector<docstring> & callers) const;
	/// Returns the value of the counter according to the
	/// numbering scheme numbertype.
	/** Available numbering schemes are arabic (1, 2,...), roman
	 *  (i, ii,...), Roman (I, II,...), alph (a, b,...), Alpha (A,
	 *  B,...) and hebrew.
	 */
	docstring labelItem(docstring const & ctr,
			    docstring const & numbertype) const;
	/// Used for managing the counter_stack_.
	// @{
	void beginEnvironment();
	void endEnvironment();
	// @}
	/// Maps counter (layout) names to actual counters.
	typedef std::map<docstring, Counter> CounterList;
	/// Instantiate.
	CounterList counterList_;
	/// Are we in an appendix?
	bool appendix_;
	/// The current enclosing float.
	std::string current_float_;
	/// Are we in a subfloat?
	bool subfloat_;
	/// Are we in a longtable?
	bool longtable_;
	/// Used to keep track of active counters.
	std::vector<docstring> counter_stack_;
	/// Same, but for last layout.
	std::vector<Layout const *> layout_stack_;
};

} // namespace lyx

#endif
