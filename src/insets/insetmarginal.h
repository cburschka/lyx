// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 *======================================================
 */

#ifndef INSETMARGINAL_H
#define INSETMARGINAL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetfootlike.h"

/** The marginal note inset

*/
class InsetMarginal : public InsetFootlike {
public:
	///
	InsetMarginal(BufferParams const &);
	///
	InsetMarginal(InsetMarginal const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::MARGIN_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
};

#endif
