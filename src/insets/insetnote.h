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
	InsetNote(InsetNote const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	/// constructor with initial contents
	InsetNote(Buffer const *, string const & contents, bool collapsed);
	///
	virtual string const editMessage() const;
	///
	virtual Inset::Code lyxCode() const { return Inset::IGNORE_CODE; }
	///
	virtual void write(Buffer const *, std::ostream &) const;
	///
	virtual int latex(Buffer const *, std::ostream &, bool, bool) const
		{ return 0; }
private:
	/// used by the constructors
	void init();

};

#endif
