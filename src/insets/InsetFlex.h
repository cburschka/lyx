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
	docstring layoutName() const override { return from_utf8("Flex:" + name_); }
	///
	InsetLayout const & getLayout() const override;
	///
	InsetCode lyxCode() const override { return FLEX_CODE; }
	/// Default looks
	InsetLayout::InsetDecoration decoration() const override;
	///
	void write(std::ostream &) const override;
	/// should paragraph indentation be omitted in any case?
	bool neverIndent() const override { return true; }
	///
	bool hasSettings() const override { return false; }
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	void updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted = false) override;

protected:
	///
	InsetFlex(InsetFlex const &);

private:
	///
	Inset * clone() const override { return new InsetFlex(*this); }
	///
	std::string name_;
};


} // namespace lyx

#endif
