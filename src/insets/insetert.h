// -*- C++ -*-
/**
 * \file insetert.h
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


#include "insetcollapsable.h"

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
	InsetERT(BufferParams const &, bool collapsed = false);
	///
	InsetERT(InsetERT const &);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	InsetERT(BufferParams const &,
		 Language const *, std::string const & contents, bool collapsed);
	///
	~InsetERT();
	///
	InsetOld::Code lyxCode() const { return InsetOld::ERT_CODE; }
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	std::string const editMessage() const;
	///
	bool insertInset(BufferView *, InsetOld *);
	///
	bool insetAllowed(InsetOld::Code code) const;
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getDrawFont(LyXFont &) const;
	///
	bool forceDefaultParagraphs(InsetOld const *) const { return true; }
	///
	void setStatus(CollapseStatus st);
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
private:
	///
	void lfunMousePress(FuncRequest const &);
	///
	// the bool return is used to see if we opened a dialog so that we can
	// check this from an outer inset and open the dialog of the outer inset
	// if that one has one!
	///
	bool lfunMouseRelease(FuncRequest const &);
	///
	void lfunMouseMotion(FuncRequest const &);
	///
	void init();
	///
	void setButtonLabel() const;
	///
	void setLatexFont(BufferView *);
	/// update status on button
	void updateStatus(bool = false) const;
	///
	void edit(BufferView * bv, bool left);
	///
	bool allowSpellCheck() const { return false; }
};


#include "mailinset.h"

class InsetERTMailer : public MailInset {
public:
	///
	InsetERTMailer(InsetERT & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
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

#endif
