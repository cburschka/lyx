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

#ifndef INSETOPTARG_H
#define INSETOPTARG_H


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
	InsetArgument(Buffer *);

	/// Outputting the parameter of a LaTeX command
	void latexArgument(otexstream &, OutputParams const &,
			bool optional) const;

	/// \name Public functions inherited from Inset class
	//@{
	///
	bool hasSettings() const { return false; }
	///
	InsetCode lyxCode() const { return ARG_CODE; }
	///
	docstring layoutName() const { return from_ascii("Argument"); }
	///
	void latex(otexstream &, OutputParams const &) const { }
	///
	int plaintext(odocstream &, OutputParams const &) const { return 0; }
	///
	int docbook(odocstream &, OutputParams const &) const { return 0; }
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const 
		{ return docstring(); };
	///
	void write(std::ostream & os) const;
	///
	bool neverIndent() const { return true; }
	//@}

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetArgument(*this); }
	//@}
};


} // namespace lyx

#endif // INSETOPTARG_H
