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


#ifndef INSETERT_H
#define INSETERT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

class Painter;

/** A colapsable text inset
  
  To write full ert (including styles and other insets) in a given
  space. 
*/
class InsetERT : public InsetCollapsable {
public:
	///
	InsetERT();
	///
	~InsetERT() {}
	///
	void Write(Buffer const * buf, ostream & os) const;
	///
	Inset * Clone() const;
	///
	char const * EditMessage() const;
	///
	bool InsertInset(BufferView *, Inset *);
	///
	void SetFont(BufferView *, LyXFont const &, bool toggleall = false);
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
};

#endif
