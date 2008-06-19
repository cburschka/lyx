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
	InsetERT(BufferParams const &, CollapseStatus status = Open);
#if 0
	///
	InsetERT(BufferParams const &,
		 Language const *, std::string const & contents, CollapseStatus status);
#endif
	///
	~InsetERT();
	///
	Inset::Code lyxCode() const { return Inset::ERT_CODE; }
	///
	docstring name() const { return from_ascii("ERT"); }
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	virtual docstring const editMessage() const;
	///
	bool insetAllowed(Inset::Code code) const;
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getDrawFont(Font &, Font) const;
	///
	bool forceDefaultParagraphs(idx_type) const { return true; }
	/// should paragraph indendation be ommitted in any case?
	bool neverIndent(Buffer const &) const { return true; }
protected:
	InsetERT(InsetERT const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const;
	///
	void init();
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
