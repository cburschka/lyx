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

#ifndef INSET_NOTE_H
#define INSET_NOTE_H

#include "InsetCollapsible.h"


namespace lyx {

class InsetNoteParams
{
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


/////////////////////////////////////////////////////////////////////////
//
// InsetNote
//
/////////////////////////////////////////////////////////////////////////

/// The PostIt note inset, and other annotations
class InsetNote : public InsetCollapsible
{
public:
	///
	InsetNote(Buffer *, std::string const &);
	///
	~InsetNote();
	///
	static std::string params2string(InsetNoteParams const &);
	///
	static void string2params(std::string const &, InsetNoteParams &);
	///
	InsetNoteParams const & params() const { return params_; }
private:
	///
	InsetCode lyxCode() const override { return NOTE_CODE; }
	///
	docstring layoutName() const override;
	/** returns false if, when outputting LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const override { return params_.type == InsetNoteParams::Note; }
	/// Is the content of this inset part of the output document?
	bool producesOutput() const override
		{ return params_.type == InsetNoteParams::Greyedout; }
	///
	bool allowSpellCheck() const override;
	///
	FontInfo getFont() const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const override;
	///
	bool isMacroScope() const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetNote(*this); }
	/// used by the constructors
	void init();
	///
	std::string contextMenuName() const override;
	///
	friend class InsetNoteParams;

	///
	InsetNoteParams params_;
};

} // namespace lyx

#endif // INSET_NOTE_H
