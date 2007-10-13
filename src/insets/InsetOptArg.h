// -*- C++ -*-
/**
 * \file InsetOptArg.h
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
 * InsetOptArg. Used to insert a short version of sectioning header etc.
 * automatically, or other optional LaTeX arguments
 */
class InsetOptArg : public InsetCollapsable {
public:
	InsetOptArg(BufferParams const &);

	/// code of the inset
	InsetCode lyxCode() const { return OPTARG_CODE; }
	/// return an message upon editing
	virtual docstring const editMessage() const;

	/// Standard LaTeX output -- short-circuited
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	/// Standard plain text output -- short-circuited
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	/// Standard DocBook output -- short-circuited
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;

	/// Outputting the optional parameter of a LaTeX command
	int latexOptional(Buffer const &, odocstream &,
			  OutputParams const &) const;
	/// Write out to the .lyx file
	void write(Buffer const & buf, std::ostream & os) const;

	/// should paragraph indendation be ommitted in any case?
	virtual bool neverIndent(Buffer const &) const { return true; }
protected:
	InsetOptArg(InsetOptArg const &);
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif // INSETOPTARG_H
