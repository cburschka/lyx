// -*- C++ -*-
/**
 * \file math_macrotable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include <map>
#include <string>
#include <vector>

class MathArray;

///
class MacroData {
public:
	///
	MacroData();
	///
	MacroData(std::string const & def, int nargs, std::string const & disp);
	///
	std::string def() const { return def_; }
	///
	std::string disp() const { return disp_; }
	///
	int numargs() const { return numargs_; }
	/// replace #1,#2,... by given MathAtom 0,1,..
	void expand(MathArray const & from, MathArray & to) const;

private:
	///
	std::string def_;	
	///
	int numargs_;
	///
	std::string disp_;
};


// This contains a table of "global" macros that are always accessible,
// either because they implement a feature of standard LaTeX or some
// hack to display certain contents nicely.

class MacroTable {
public:
	/// Parse full "\def..." or "\newcommand..." or ...
	void insert(std::string const & definition);
	/// Insert pre-digested macro definition
	void insert(std::string const & name, MacroData const & data);
	/// Do we have a macro by that name?
	bool has(std::string const & name) const;
	///
	MacroData const & get(std::string const & name) const;
	///
	void dump();

	/// the global list
	static MacroTable & globalMacros();

private:
	///
	typedef std::map<std::string, MacroData> table_type;

	///
	table_type macros_;
};

#endif
