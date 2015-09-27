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

#include "InsetCollapsable.h"


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
class InsetNote : public InsetCollapsable
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
	InsetCode lyxCode() const { return NOTE_CODE; }
	///
	docstring layoutName() const;
	///
	DisplayType display() const;
	/** returns false if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const { return params_.type == InsetNoteParams::Note; }
	/// Is the content of this inset part of the output document?
	bool producesOutput() const
		{ return params_.type == InsetNoteParams::Greyedout; }
	///
	bool allowSpellCheck() const;
	///
	FontInfo getFont() const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	/// show the note dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	bool isMacroScope() const;
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetNote(*this); }
	/// used by the constructors
	void init();
	///
	std::string contextMenuName() const;
	///
	friend class InsetNoteParams;

	///
	InsetNoteParams params_;
};

} // namespace lyx

#endif // INSET_NOTE_H
