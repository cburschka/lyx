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
// The pristine updatable inset: Text


#ifndef INSETFOOT_H
#define INSETFOOT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetfootlike.h"

/** The footnote inset
  
*/
class InsetFoot : public InsetFootlike {
public:
	///
	InsetFoot();
	///
	Inset * Clone(Buffer const &) const;
	///
	Inset::Code LyxCode() const { return Inset::FOOT_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
};

#endif



