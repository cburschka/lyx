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
	InsetMarginal();
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MARGIN_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	const char * EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
};

#endif
