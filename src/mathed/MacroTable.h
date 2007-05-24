// -*- C++ -*-
/**
 * \file MacroTable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include "support/docstring.h"

#include <boost/assert.hpp>

#include <map>
#include <vector>

namespace lyx {

class MathData;

///
class MacroData {
public:
	///
	MacroData();
	///
	MacroData(docstring const & def, int nargs, docstring const & disp, std::string const &);
	///
	docstring def() const { return def_; }
	///
	docstring disp() const { return disp_; }
	///
	int numargs() const { return numargs_; }
	/// replace #1,#2,... by given MathAtom 0,1,..
	void expand(std::vector<MathData> const & from, MathData & to) const;
	///
	std::string requires() const { return requires_; }
	///
	std::string & requires() { return requires_; }
	/// lock while being drawn
	int lock() const { return ++lockCount_; }
	/// is it being drawn?
	bool locked() const { return lockCount_ != 0; }
	///
	void unlock() const { --lockCount_; BOOST_ASSERT(lockCount_ >= 0); } 	

	///
	bool operator==(MacroData const & x) const {
		return def_ == x.def_ &&
			numargs_ == x.numargs_ &&
			disp_ == x.disp_ &&
			requires_ == x.requires_;
	}
	///
	bool operator!=(MacroData const & x) const { return !operator==(x); }
	
private:
	///
	docstring def_;
	///
	int numargs_;
	///
	docstring disp_;
	///
	std::string requires_;
	///
	mutable int lockCount_;
};


// This contains a table of "global" macros that are always accessible,
// either because they implement a feature of standard LaTeX or some
// hack to display certain contents nicely.

class MacroTable : public std::map<docstring, MacroData>
{
public:
	/// Parse full "\\def..." or "\\newcommand..." or ...
	void insert(docstring const & definition, std::string const &);
	/// Insert pre-digested macro definition
	void insert(docstring const & name, MacroData const & data);
	/// Do we have a macro by that name?
	bool has(docstring const & name) const;
	///
	MacroData const & get(docstring const & name) const;
	///
	void dump();

	/// the global list
	static MacroTable & globalMacros();
	/// the local list hack
	//static MacroTable & localMacros();
};


} // namespace lyx

#endif
