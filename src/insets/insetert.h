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
	InsetERT(string const & contents);
	///
	virtual void write(Buffer const * buf, std::ostream & os) const;
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;
	///
	virtual string const editMessage() const;
	///
	virtual bool insertInset(BufferView *, Inset *);
	///
	virtual bool insetAllowed(Inset::Code) const { return false; }
	///
	virtual void setFont(BufferView *, LyXFont const &,
			     bool toggleall = false, bool selectall = false);
	///
	virtual void edit(BufferView *, int, int, unsigned int);
	///
	virtual int latex(Buffer const *, std::ostream &, bool fragile,
			  bool free_spc) const;
	///
	virtual int ascii(Buffer const *,
			  std::ostream &, int linelen = 0) const;
	///
	virtual int linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int docBook(Buffer const *, std::ostream &) const;
};

#endif
