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
	Inset::Code lyxCode() const { return Inset::FOOT_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
};

#endif



