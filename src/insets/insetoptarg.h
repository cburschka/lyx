// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team
 *
 * ====================================================== */

#ifndef INSETOPTARG_H
#define INSETOPTARG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"
#include "insetcollapsable.h"

class InsetOptArg : public InsetCollapsable {
public:
	InsetOptArg(BufferParams const &);
	///
	InsetOptArg(InsetOptArg const &, bool same_id = false);

	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::OPTARG_CODE; }
	///
	string const editMessage() const;
	/// Standard LaTeX output -- short-circuited
	int latex(Buffer const *, std::ostream &, 
					bool fragile, bool fp) const;
	/// Outputting the optional parameter of a LaTeX command
	int latexOptional(Buffer const *, std::ostream &, 
					bool fragile, bool fp) const;
	///
	void write(Buffer const * buf, ostream & os) const;
};

#endif
