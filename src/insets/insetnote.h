// -*- C++ -*-
/**
 * \file insetnote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETNOTE_H
#define INSETNOTE_H


#include "insetcollapsable.h"


 struct InsetNoteParams {
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	string type;
};


/** The PostIt note inset, and other annotations

*/
class InsetNote : public InsetCollapsable {
public:
	///


	InsetNote(BufferParams const &, string const &);
	/// Copy constructor
	InsetNote(InsetNote const &);
	///
	~InsetNote();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	string const editMessage() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::NOTE_CODE; }
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void setButtonLabel();
	///
	dispatch_result InsetNote::localDispatch(FuncRequest const &);
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	int latex(Buffer const *, std::ostream &,
			LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool) const;
	///
	int ascii(Buffer const *, std::ostream &, int) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetNoteParams const & params() const { return params_; }

private:
	friend class InsetNoteParams;

	/// used by the constructors
	void init();
	///
	InsetNoteParams params_;
};

#include "mailinset.h"

class InsetNoteMailer : public MailInset {
public:
	///
	InsetNoteMailer(string const & name, InsetNote & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string(Buffer const &) const;
	///
	static string const params2string(string const &, InsetNoteParams const &);
	///
	static void string2params(string const &, InsetNoteParams &);

private:
	///
	string const name_;
	///
	InsetNote & inset_;
};



#endif
