// -*- C++ -*-
/**
 * \file insetoptarg.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Martin Vermeer  <martin.vermeer@hut.fi>
 */

#ifndef INSETOPTARG_H
#define INSETOPTARG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"
#include "insetcollapsable.h"

/**
 * InsetOptArg. Used to insert a short version of sectioning header etc.
 * automatically, or other optional LaTeX arguments
 */
class InsetOptArg : public InsetCollapsable {
public:
	InsetOptArg(BufferParams const &);

	InsetOptArg(InsetOptArg const &, bool same_id = false);

	/// make a duplicate of this inset
	Inset * clone(Buffer const &, bool same_id = false) const;
 
	/// this inset is editable
	EDITABLE editable() const { return IS_EDITABLE; }
	/// code of the inset
	Inset::Code lyxCode() const { return Inset::OPTARG_CODE; }
	/// return an message upon editing
	string const editMessage() const;
 
	/// Standard LaTeX output -- short-circuited
	int latex(Buffer const *, std::ostream &,
					bool fragile, bool fp) const;
	/// Outputting the optional parameter of a LaTeX command
	int latexOptional(Buffer const *, std::ostream &,
					bool fragile, bool fp) const;
	/// Write out tothe .lyx file
	void write(Buffer const * buf, ostream & os) const;
};

#endif // INSETOPTARG_H
