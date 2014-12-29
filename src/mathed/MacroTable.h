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

#include "DocIterator.h"

#include "support/docstring.h"

#include <map>
#include <set>
#include <vector>

namespace lyx {

class Buffer;
class MathData;
class MathMacroTemplate;
class Paragraph;
class latexkeys;

enum MacroType {
	MacroTypeNewcommand,
	MacroTypeNewcommandx,
	MacroTypeDef
};

///
class MacroData {
public:
	/// Constructor to make STL containers happy
	MacroData(Buffer * buf = 0);
	/// Create lazy MacroData which only queries the macro template when needed
	MacroData(Buffer * buf, DocIterator const & pos);
	/// Create non-lazy MacroData which directly queries the macro template
	MacroData(Buffer * buf, MathMacroTemplate const & macro);

	///
	docstring const & definition() const { updateData(); return definition_; }
	///
	docstring const & display() const { updateData(); return display_; }
	/// arity including optional arguments (if there is any)
	size_t numargs() const { updateData(); return numargs_; }
	/// replace #1,#2,... by given MathAtom 0,1,.., _including_ the possible
	/// optional argument
	/// \return whether anything was expanded
	bool expand(std::vector<MathData> const & from, MathData & to) const;
	/// number of optional arguments
	size_t optionals() const;
	///
	std::vector<docstring> const & defaults() const;
	///
	std::string const requires() const;
	///
	void setSymbol(latexkeys const * sym) { sym_ = sym; }

	/// lock while being drawn to avoid recursions
	int lock() const { return ++lockCount_; }
	/// is it being drawn?
	bool locked() const { return lockCount_ != 0; }
 	///
	void unlock() const;

	///
	bool redefinition() const { updateData(); return redefinition_; }

	///
	MacroType type() const { updateData(); return type_; }

	/// output as TeX macro, only works for lazy MacroData!!!
	int write(odocstream & os, bool overwriteRedefinition) const;

	///
	bool operator==(MacroData const & x) const {
		updateData();
		x.updateData();
		return definition_ == x.definition_
			&& numargs_ == x.numargs_
			&& display_ == x.display_
			&& sym_ == x.sym_
			&& optionals_ == x.optionals_
			&& defaults_ == x.defaults_;
	}
	///
	bool operator!=(MacroData const & x) const { return !operator==(x); }

private:
	///
	void queryData(MathMacroTemplate const & macro) const;
	///
	void updateData() const;
	///
	Buffer const * buffer_;
	/// The position of the definition in the buffer.
	/// There is no guarantee it stays valid if the buffer
	/// changes. But it (normally) exists only until the
	/// next Buffer::updateMacros call where new MacroData
	/// objects are created for each macro definition.
	/// In the worst case, it is invalidated and the MacroData
	/// returns its defaults values and the user sees unfolded
	/// macros.
	mutable DocIterator pos_;
	///
	mutable bool queried_;
	///
	mutable docstring definition_;
	///
	mutable size_t numargs_;
	///
	mutable docstring display_;
	///
	latexkeys const * sym_;
	///
	mutable size_t optionals_;
	///
	mutable std::vector<docstring> defaults_;
	///
	mutable int lockCount_;
	///
	mutable bool redefinition_;
	///
	mutable MacroType type_;
};


///
class MacroNameSet : public std::set<docstring> {};
///
class MacroSet : public std::set<MacroData const *> {};


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
	iterator insert(Buffer * buf, docstring const & definition);
	/// Insert pre-digested macro definition
	iterator insert(docstring const & name, MacroData const & data);
	///
	MacroData const * get(docstring const & name) const;
	///
	void dump();
	///
	void getMacroNames(std::set<docstring> & names) const;

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
	/// construct context for the insets at pos
	MacroContext(Buffer const * buf, DocIterator const & pos);

	/// Lookup macro
	MacroData const * get(docstring const & name) const;

private:
	///
	Buffer const * buf_;
	///
	DocIterator const & pos_;
};

} // namespace lyx

#endif
