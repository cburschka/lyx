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

/** A colapsable text inset
  
  To write full ert (including styles and other insets) in a given
  space. 
*/
class InsetERT : public InsetCollapsable {
public:
	///
	InsetERT();
	///
	virtual void Write(Buffer const * buf, std::ostream & os) const;
	///
	virtual Inset * Clone(Buffer const &) const;
	///
	virtual string const EditMessage() const;
	///
	virtual bool InsertInset(BufferView *, Inset *);
	///
	virtual void SetFont(BufferView *, LyXFont const &,
			     bool toggleall = false, bool selectall = false);
	///
	virtual void Edit(BufferView *, int, int, unsigned int);
	///
	virtual int Latex(Buffer const *, std::ostream &, bool fragile,
			  bool free_spc) const;
	///
	virtual int Ascii(Buffer const *,
			  std::ostream &, int linelen = 0) const;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const;
};

#endif
