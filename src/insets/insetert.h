// -*- C++ -*-
/**
 * \file insetert.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
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
	enum ERTStatus {
		Open,
		Collapsed,
		Inlined
	};
	///
	InsetERT(BufferParams const &, bool collapsed = false);
	///
	InsetERT(InsetERT const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	InsetERT(BufferParams const &,
		 Language const *, string const & contents, bool collapsed);
	///
	~InsetERT();
	///
	Inset::Code lyxCode() const { return Inset::ERT_CODE; }
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	string const editMessage() const;
	///
	bool insertInset(BufferView *, Inset *);
	///
	bool insetAllowed(Inset::Code code) const { return code == Inset::NEWLINE_CODE; }
	///
	void setFont(BufferView *, LyXFont const &,
			     bool toggleall = false, bool selectall = false);
	///
	EDITABLE editable() const;
	///
	int latex(Buffer const *, std::ostream &, bool fragile,
		  bool free_spc) const;
	///
	int ascii(Buffer const *,
			  std::ostream &, int linelen = 0) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	RESULT localDispatch(FuncRequest const &);
	///
	bool checkInsertChar(LyXFont &);
	///
	// this are needed here because of the label/inlined functionallity
	///
	bool needFullRow() const { return status_ == Open; }
	///
	bool isOpen() const { return status_ == Open || status_ == Inlined; }
	///
	bool inlined() const { return status_ == Inlined; }
	///
	ERTStatus status() const { return status_; }
	///
	void open(BufferView *);
	///
	void close(BufferView *) const;
	///
	bool allowSpellcheck() const { return false; }

	WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float &) const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, const LyXFont &, int , float &) const;
	/// set the status of the inset
	void status(BufferView *, ERTStatus const st) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getDrawFont(LyXFont &) const;
	///
	bool forceDefaultParagraphs(Inset const *) const {
		return true;
	}
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	void update(BufferView *, bool =false);

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
	string const get_new_label() const;
	///
	void setButtonLabel() const;
	///
	void set_latex_font(BufferView *);
	/// update status on button
	void updateStatus(BufferView *, bool = false) const;

	///
	mutable ERTStatus status_;
};


#include "mailinset.h"

class InsetERTMailer : public MailInset {
public:
	///
	InsetERTMailer(InsetERT & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string() const;
	///
	static void string2params(string const &, InsetERT::ERTStatus &);
	///
	static string const params2string(InsetERT::ERTStatus);
private:
	///
	static string const name_;
	///
	InsetERT & inset_;
};

#endif
