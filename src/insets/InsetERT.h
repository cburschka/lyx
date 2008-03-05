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
#include "MailInset.h"


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
	bool forceEmptyLayout() const { return true; }
	///
	bool allowParagraphCustomization(idx_type) const { return false; }
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


class InsetERTMailer : public MailInset {
public:
	///
	InsetERTMailer(InsetERT & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &,
		InsetCollapsable::CollapseStatus &);
	///
	static std::string const params2string(InsetCollapsable::CollapseStatus);
private:
	///
	static std::string const name_;
	///
	InsetERT & inset_;
};


} // namespace lyx

#endif
