// -*- C++ -*-
/**
 * \file insetoptarg.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETOPTARG_H
#define INSETOPTARG_H


#include "insetcollapsable.h"


/**
 * InsetOptArg. Used to insert a short version of sectioning header etc.
 * automatically, or other optional LaTeX arguments
 */
class InsetOptArg : public InsetCollapsable {
public:
	InsetOptArg(BufferParams const &);

	InsetOptArg(InsetOptArg const &);
	/// make a duplicate of this inset
	virtual std::auto_ptr<InsetBase> clone() const;
	/// this inset is editable
	EDITABLE editable() const { return IS_EDITABLE; }
	/// code of the inset
	InsetOld::Code lyxCode() const { return InsetOld::OPTARG_CODE; }
	/// return an message upon editing
	std::string const editMessage() const;

	/// Standard LaTeX output -- short-circuited
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	/// Outputting the optional parameter of a LaTeX command
	int latexOptional(Buffer const &, std::ostream &,
			  LatexRunParams const &) const;
	/// Write out tothe .lyx file
	void write(Buffer const & buf, std::ostream & os) const;
};

#endif // INSETOPTARG_H
