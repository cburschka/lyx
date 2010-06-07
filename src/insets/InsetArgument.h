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
	int latexArgument(odocstream &, OutputParams const &,
			bool optional) const;
	///
	bool hasSettings() const { return false; }

private:
	/// code of the inset
	InsetCode lyxCode() const { return ARG_CODE; }
	///
	docstring name() const { return from_ascii("Argument"); }
	/// Standard LaTeX output -- short-circuited
	int latex(odocstream &, OutputParams const &) const;
	/// Standard plain text output -- short-circuited
	int plaintext(odocstream &, OutputParams const &) const;
	/// Standard DocBook output -- short-circuited
	int docbook(odocstream &, OutputParams const &) const;
	/// Standard XHTML output -- short-circuited
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	/// Write out to the .lyx file
	void write(std::ostream & os) const;
	/// should paragraph indendation be ommitted in any case?
	bool neverIndent() const { return true; }
	///
	Inset * clone() const { return new InsetArgument(*this); }
};


} // namespace lyx

#endif // INSETOPTARG_H
