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
	InsetERT(Buffer const &, CollapseStatus status = Open);
	///
	~InsetERT();
	///
	static CollapseStatus string2params(std::string const &);
	///
	static std::string params2string(CollapseStatus);
private:
	///
	InsetCode lyxCode() const { return ERT_CODE; }
	///
	docstring name() const { return from_ascii("ERT"); }
	///
	void write(std::ostream & os) const;
	///
	docstring editMessage() const;
	///
	bool insetAllowed(InsetCode code) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	virtual bool forceEmptyLayout(idx_type = 0) const { return true; }
	///
	virtual bool allowParagraphCustomization(idx_type = 0) const { return false; }
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetERT(*this); }
	///
	void setButtonLabel();
	///
	bool allowSpellCheck() const { return false; }
};


} // namespace lyx

#endif
