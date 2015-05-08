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


#include "InsetCollapsable.h"


namespace lyx {


/**
 * InsetArgument. Used to insert a short version of sectioning header etc.
 * automatically, or other optional LaTeX arguments
 */
class InsetArgument : public InsetCollapsable
{
public:
	///
	InsetArgument(Buffer *, std::string const &);

	/// Outputting the parameter of a LaTeX command
	void latexArgument(otexstream &, OutputParams const &,
			   docstring const&, docstring const &,
			   docstring const &) const;

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
	void updateBuffer(ParIterator const &, UpdateType);
	///
	void latex(otexstream &, OutputParams const &) const { }
	///
	int plaintext(odocstringstream &, OutputParams const &, size_t) const { return 0; }
	///
	int docbook(odocstream &, OutputParams const &) const { return 0; }
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const 
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
	bool resetFontEdit() const { return false; }
	//@}
	/// \name Public functions inherited from InsetCollapsable class
	//@{
	///
	InsetLayout::InsetDecoration decoration() const;
	///
	FontInfo getFont() const;
	///
	FontInfo getLabelfont() const;
	///
	void setButtonLabel();
	//@}

private:
	///
	docstring toolTip(BufferView const & bv, int, int) const;
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
	///
	bool pass_thru_;
	///
	docstring pass_thru_chars_;

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetArgument(*this); }
	//@}
};


} // namespace lyx

#endif // INSETARGUMENT_H
