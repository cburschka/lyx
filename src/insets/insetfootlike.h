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

/** The footnote inset
  
*/
class InsetFootlike : public InsetCollapsable {
public:
	///
	InsetFootlike();
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	//LyXFont GetDrawFont(BufferView *, LyXParagraph * par, int pos) const;
};

#endif





