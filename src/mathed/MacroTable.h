// -*- C++ -*-
/**
 * \file MacroTable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include "support/docstring.h"

#include <map>
#include <vector>

namespace lyx {

class Buffer;
class MathData;
class Paragraph;

///
class MacroData {
public:
	///
	MacroData();
	///
	MacroData(docstring const & definition,
		std::vector<docstring> const & defaults, int nargs, int optionals, 
		docstring const & display, std::string const & requires);
	///
	docstring const & definition() const { return definition_; }
	///
	docstring const & display() const { return display_; }
	/// arity including optional arguments (if there is any)
	size_t numargs() const { return numargs_; }
	/// replace #1,#2,... by given MathAtom 0,1,.., _including_ the possible
	/// optional argument
	void expand(std::vector<MathData> const & from, MathData & to) const;
	/// number of optional arguments
	size_t optionals() const;
	///
	std::vector<docstring> const & defaults() const;
	///
	std::string requires() const { return requires_; }
	///
	std::string & requires() { return requires_; }
	
	/// lock while being drawn to avoid recursions
	int lock() const { return ++lockCount_; }
	/// is it being drawn?
	bool locked() const { return lockCount_ != 0; }
 	///
	void unlock() const;
	
	///
	bool redefinition() const { return redefinition_; }
	///
	void setRedefinition(bool redefined) { redefinition_ = redefined; }
	
	///
	bool operator==(MacroData const & x) const {
		return definition_ == x.definition_ 
			&& numargs_ == x.numargs_
			&& display_ == x.display_
			&& requires_ == x.requires_
			&& optionals_ == x.optionals_
			&& defaults_ == x.defaults_;
	}
	///
	bool operator!=(MacroData const & x) const { return !operator==(x); }

private:
	///
	docstring definition_;
	///
	size_t numargs_;
	///
	docstring display_;
	///
	std::string requires_;
	///
	mutable int lockCount_;
	///
	bool redefinition_;
	///
	size_t optionals_;
	///
	std::vector<docstring> defaults_;
};


/// A lookup table of macro definitions.
/**
 * This contains a table of "global" macros that are always accessible,
 * either because they implement a feature of standard LaTeX or some
 * hack to display certain contents nicely.
 *
 **/
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
};


/// A context to lookup macros at a certain position in a buffer.
/**
 * The MacroContext is used during metrics calculation to resolve
 * macro instances according to the position of them in the buffer
 * document. Only macro definition in front of the macro instance
 * are visible and are resolved.
 *
 **/
class MacroContext {
public:
	/// construct context for insets in par (not including the ones
	/// defined in par itself)
	MacroContext(Buffer const & buf, Paragraph const & par);
	
	/// Look for macro
	bool has(docstring const & name) const;
	/// Lookup macro
	MacroData const & get(docstring const & name) const;
	
	/// Insert pre-digested macro definition
	void insert(docstring const & name, MacroData const & data);
	
private:
	/// context local macros
	MacroTable macros_;
	///
	Buffer const & buf_;
	///
	Paragraph const & par_;
};

} // namespace lyx

#endif
