// -*- C++ -*-
/**
 * \file insetnote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETNOTE_H
#define INSETNOTE_H

#include "insetcollapsable.h"


struct InsetNoteParams {
	enum Type {
		Note,
		Comment,
		Greyedout
	};
	/// \c type defaults to Note
	InsetNoteParams();
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	Type type;
};


/** The PostIt note inset, and other annotations

*/
class InsetNote : public InsetCollapsable {
public:
	///
	InsetNote(BufferParams const &, std::string const &);
	/// Copy constructor
	InsetNote(InsetNote const &);
	///
	~InsetNote();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	std::string const editMessage() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::NOTE_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void setButtonLabel();
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	int latex(Buffer const &, std::ostream &,
	       OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
	       OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
	       OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
	       OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetNoteParams const & params() const { return params_; }
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(BufferView & bv, FuncRequest const & cmd);
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
	InsetNoteMailer(InsetNote & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static std::string const params2string(InsetNoteParams const &);
	///
	static void string2params(std::string const &, InsetNoteParams &);

private:
	///
	static std::string const name_;
	///
	InsetNote & inset_;
};

#endif
