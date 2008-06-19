// -*- C++ -*-
/**
 * \file InsetNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETNOTE_H
#define INSETNOTE_H

#include "InsetCollapsable.h"
#include "MailInset.h"


namespace lyx {

class InsetNoteParams {
public:
	enum Type {
		Note,
		Comment,
		Greyedout,
		Framed,
		Shaded
	};
	/// \c type defaults to Note
	InsetNoteParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Type type;
};


/** The PostIt note inset, and other annotations

*/
class InsetNote : public InsetCollapsable {
public:
	///
	InsetNote(BufferParams const &, std::string const &);
	///
	~InsetNote();
	///
	virtual docstring const editMessage() const;
	///
	Inset::Code lyxCode() const { return Inset::NOTE_CODE; }
	///
	docstring name() const { return from_ascii("Note"); }
	/// framed and shaded notes are displayed
	virtual DisplayType display() const;
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	void setButtonLabel();
	///
	virtual Color_color backgroundColor() const;
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetNoteParams const & params() const { return params_; }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	bool noFontChange() const { return params_.type != InsetNoteParams::Note; }
protected:
	InsetNote(InsetNote const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	friend class InsetNoteParams;

	virtual std::auto_ptr<Inset> doClone() const;

	/// used by the constructors
	void init();
	///
	InsetNoteParams params_;
};


class InsetNoteMailer : public MailInset {
public:
	///
	InsetNoteMailer(InsetNote & inset);
	///
	virtual Inset & inset() const { return inset_; }
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


} // namespace lyx

#endif
