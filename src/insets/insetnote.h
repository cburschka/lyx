// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 *======================================================
 */

#ifndef INSETNOTE_H
#define INSETNOTE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** The PostIt note inset
  
*/
class InsetNote : public InsetCollapsable {
public:
	///
	InsetNote();
	///
	virtual Inset * clone(Buffer const &, bool) const;
	///
	virtual string const editMessage() const;
	///
	virtual Inset::Code lyxCode() const { return Inset::IGNORE_CODE; }
	///
	virtual bool insertInset(BufferView *, Inset *) { return false; }
	///
	virtual bool insetAllowed(Inset::Code) const { return false; }
	///
	virtual void write(Buffer const *, std::ostream &) const;
	///
	virtual int latex(Buffer const *, std::ostream &, bool, bool) const
		{ return 0; }
};

#endif
