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
	InsetERT(string const & contents, bool collapsed);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	string const editMessage() const;
	///
	bool insertInset(BufferView *, Inset *);
	///
	bool insetAllowed(Inset::Code) const { return false; }
	///
	void setFont(BufferView *, LyXFont const &,
	                     bool toggleall = false, bool selectall = false);
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	int latex(Buffer const *, std::ostream &, bool fragile,
	                  bool free_spc) const;
	///
	int ascii(Buffer const *,
	                  std::ostream &, int linelen = 0) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docBook(Buffer const *, std::ostream &) const;
	///
	UpdatableInset::RESULT localDispatch(BufferView *, kb_action,
	                                     string const &);
private:
	///
	void init();
};

#endif
