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

	/// code of the inset
	InsetBase::Code lyxCode() const { return InsetBase::OPTARG_CODE; }
	/// return an message upon editing
	std::string const editMessage() const;

	/// Standard LaTeX output -- short-circuited
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	/// Outputting the optional parameter of a LaTeX command
	int latexOptional(Buffer const &, std::ostream &,
			  OutputParams const &) const;
	/// Write out tothe .lyx file
	void write(Buffer const & buf, std::ostream & os) const;
protected:
	InsetOptArg(InsetOptArg const &);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif // INSETOPTARG_H
