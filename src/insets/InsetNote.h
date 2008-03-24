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
		Greyedout
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
	InsetNote(Buffer const &, std::string const &);
	///
	~InsetNote();
	///
	InsetNoteParams const & params() const { return params_; }
private:
	///
	docstring editMessage() const;
	///
	InsetCode lyxCode() const { return NOTE_CODE; }
	///
	docstring name() const;
	///
	virtual DisplayType display() const;
	///
	bool noFontChange() const { return params_.type != InsetNoteParams::Note; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	void setButtonLabel();
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	bool isMacroScope() const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	void addToToc(ParConstIterator const &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetNote(*this); }
	/// used by the constructors
	void init();
	///
	virtual docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	friend class InsetNoteParams;

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
