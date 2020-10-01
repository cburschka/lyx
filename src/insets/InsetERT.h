// -*- C++ -*-
/**
 * \file InsetERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETERT_H
#define INSETERT_H

#include "InsetCollapsible.h"

namespace lyx {

/** A collapsible text inset for LaTeX insertions.

  To write full ert (including styles and other insets) in a given
  space.

  Note that collapsed_ encompasses both the inline and collapsed button
  versions of this inset.
*/

class Language;

class InsetERT : public InsetCollapsible {
public:
	///
	InsetERT(Buffer *, CollapseStatus status = Open);
	///
	InsetERT(InsetERT const & old);
	///
	static CollapseStatus string2params(std::string const &);
	///
	static std::string params2string(CollapseStatus);

	std::string contextMenuName() const override
		{ return "context-ert"; }
private:
	///
	InsetCode lyxCode() const override { return ERT_CODE; }
	///
	docstring layoutName() const override { return from_ascii("ERT"); }
	///
	void write(std::ostream & os) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures &) const override {}
	/// should paragraph indentation be omitted in any case?
	bool neverIndent() const override { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
 	///
	Inset * clone() const override { return new InsetERT(*this); }
	///
	docstring const buttonLabel(BufferView const & bv) const override;
	///
	bool allowSpellCheck() const override { return false; }
	///
	bool insetAllowed(InsetCode code) const override { return code == QUOTE_CODE; }
};


} // namespace lyx

#endif
