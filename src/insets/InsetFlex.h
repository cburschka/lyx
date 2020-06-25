// -*- C++ -*-
/**
 * \file InsetFlex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFLEX_H
#define INSETFLEX_H

#include "InsetCollapsible.h"

namespace lyx {

/** The Flex inset, e.g., CharStyle, Custom inset or XML short element

*/
class InsetFlex : public InsetCollapsible {
public:
	///
	InsetFlex(Buffer *, std::string const & layoutName);
	///
	docstring layoutName() const { return from_utf8("Flex:" + name_); }
	///
	InsetLayout const & getLayout() const;
	///
	InsetCode lyxCode() const { return FLEX_CODE; }
	/// Default looks
	InsetLayout::InsetDecoration decoration() const;
	///
	void write(std::ostream &) const;
	/// should paragraph indentation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	bool hasSettings() const { return false; }
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	void updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted = false);

protected:
	///
	InsetFlex(InsetFlex const &);

private:
	///
	Inset * clone() const { return new InsetFlex(*this); }
	///
	std::string name_;
};


} // namespace lyx

#endif
