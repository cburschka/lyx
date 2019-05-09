// -*- C++ -*-
/**
 * \file InsetArgument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETARGUMENT_H
#define INSETARGUMENT_H


#include "InsetCollapsible.h"


namespace lyx {


/**
 * InsetArgument. Used to insert a short version of sectioning header etc.
 * automatically, or other optional LaTeX arguments
 */
class InsetArgument : public InsetCollapsible
{
public:
	///
	InsetArgument(Buffer *, std::string const &);

	///
	InsetArgument const * asInsetArgument() const { return this; }

	/// Outputting the parameter of a LaTeX command
	void latexArgument(otexstream & os, OutputParams const & runparams_in,
	                   docstring const & ldelim, docstring const & rdelim,
	                   docstring const & presetarg) const;

	std::string name() const { return name_; }

	/// \name Public functions inherited from Inset class
	//@{
	///
	bool hasSettings() const { return false; }
	///
	InsetCode lyxCode() const { return ARG_CODE; }
	///
	docstring layoutName() const { return from_ascii("Argument"); }
	/// Update the label string of this inset
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false);
	///
	void latex(otexstream &, OutputParams const &) const { }
	///
	int plaintext(odocstringstream &, OutputParams const &, size_t) const { return 0; }
	///
	int docbook(odocstream &, OutputParams const &) const { return 0; }
	///
	docstring xhtml(XMLStream &, OutputParams const &) const
		{ return docstring(); }
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	bool neverIndent() const { return true; }
	///
	std::string contextMenuName() const;
	///
	bool isPassThru() const { return pass_thru_; }
	///
	bool isFreeSpacing() const { return free_spacing_; }
	///
	bool isTocCaption() const { return is_toc_caption_; }
	///
	bool resetFontEdit() const { return false; }
	//@}
	/// \name Public functions inherited from InsetCollapsible class
	//@{
	///
	InsetLayout::InsetDecoration decoration() const;
	///
	FontInfo getFont() const;
	///
	FontInfo getLabelfont() const;
	///
	ColorCode labelColor() const;
	///
	void setButtonLabel();
	//@}
	///
	void addToToc(DocIterator const & dit, bool output_active,
	              UpdateType utype, TocBackend & backend) const; //override

private:
	///
	docstring toolTip(BufferView const & bv, int, int) const;
	///
	void fixParagraphLanguage(Language const *);
	///
	std::string name_;
	///
	docstring labelstring_;
	///
	docstring tooltip_;
	///
	FontInfo font_;
	///
	FontInfo labelfont_;
	///
	std::string decoration_;
	/// Are we in a pass-thru context?
	bool pass_thru_context_;
	/// Is the argument itself have an explicitly pass-thru?
	bool pass_thru_local_;
	/// Effective pass-thru setting (inherited or local)
	bool pass_thru_;
	///
	bool free_spacing_;
	///
	docstring pass_thru_chars_;
	/// Does this argument provide content for the TOC?
	bool is_toc_caption_;
	/// The type of Toc this is the caption of, empty otherwise.
	std::string caption_of_toc_;
	/// Specific line break macro
	std::string newline_cmd_;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetArgument(*this); }
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const { return false; }
	//@}
};


} // namespace lyx

#endif // INSETARGUMENT_H
