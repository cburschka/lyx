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

#include "InsetCollapsable.h"


namespace lyx {

/** A collapsable text inset for LaTeX insertions.

  To write full ert (including styles and other insets) in a given
  space.

  Note that collapsed_ encompasses both the inline and collapsed button
  versions of this inset.
*/

class Language;

class InsetERT : public InsetCollapsable {
public:
	///
	InsetERT(Buffer *, CollapseStatus status = Open);
	///
	static CollapseStatus string2params(std::string const &);
	///
	static std::string params2string(CollapseStatus);

	std::string contextMenuName() const
		{ return "context-ert"; }
private:
	///
	InsetCode lyxCode() const { return ERT_CODE; }
	///
	docstring layoutName() const { return from_ascii("ERT"); }
	///
	void write(std::ostream & os) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const {}
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetERT(*this); }
	///
	docstring const buttonLabel(BufferView const & bv) const;
	///
	bool allowSpellCheck() const { return false; }
};


} // namespace lyx

#endif
