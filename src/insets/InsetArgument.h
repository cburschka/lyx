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
	InsetArgument const * asInsetArgument() const override { return this; }

	/// Outputting the parameter of a LaTeX command
	void latexArgument(otexstream & os, OutputParams const & runparams_in,
	                   docstring const & ldelim, docstring const & rdelim,
	                   docstring const & presetarg) const;

	std::string name() const { return name_; }

	bool docbookargumentbeforemaintag() const { return docbookargumentbeforemaintag_; }

	/// \name Public functions inherited from Inset class
	//@{
	///
	bool hasSettings() const override { return false; }
	///
	InsetCode lyxCode() const override { return ARG_CODE; }
	///
	docstring layoutName() const override { return from_ascii("Argument"); }
	/// Update the label string of this inset
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void latex(otexstream &, OutputParams const &) const override { }
	///
	int plaintext(odocstringstream &, OutputParams const &, size_t) const override { return 0; }
	///
	void docbook(XMLStream & xs, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override
		{ return docstring(); }
	///
	void write(std::ostream & os) const override;
	///
	void read(Lexer & lex) override;
	///
	bool neverIndent() const override { return true; }
	///
	std::string contextMenuName() const override;
	///
	bool isPassThru() const override { return pass_thru_; }
	///
	bool isFreeSpacing() const override { return free_spacing_; }
	///
	bool isTocCaption() const { return is_toc_caption_; }
	///
	bool resetFontEdit() const override { return false; }
	//@}
	/// \name Public functions inherited from InsetCollapsible class
	//@{
	///
	InsetLayout::InsetDecoration decoration() const override;
	///
	FontInfo getFont() const override;
	///
	FontInfo getLabelfont() const override;
	///
	ColorCode labelColor() const override;
	///
	void setButtonLabel() override;
	//@}
	///
	void addToToc(DocIterator const & dit, bool output_active,
	              UpdateType utype, TocBackend & backend) const override;

private:
	///
	docstring toolTip(BufferView const & bv, int, int) const override;
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
	/// DocBook tag for this argument, if any (otherwise, NONE).
	docstring docbooktag_;
	/// Type of DocBook tag (controls how new lines are inserted around this argument).
	docstring docbooktagtype_;
	/// DocBook attributes.
	docstring docbookattr_;
	///
	bool docbookargumentbeforemaintag_ = false;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetArgument(*this); }
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return false; }
	//@}
};


} // namespace lyx

#endif // INSETARGUMENT_H
