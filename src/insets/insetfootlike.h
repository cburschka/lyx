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


#ifndef InsetFootlike_H
#define InsetFootlike_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

// To have this class is probably a bit overkill... (Lgb)

/** The footnote inset
  
*/
class InsetFootlike : public InsetCollapsable {
public:
	///
	InsetFootlike(BufferParams const &);
	///
	InsetFootlike(InsetFootlike const &, bool same_id = false);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	bool insetAllowed(Inset::Code) const;
};

#endif





