// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#ifndef InsetTheorem_H
#define InsetTheorem_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** The theorem inset
  
*/
class InsetTheorem : public InsetCollapsable {
public:
	///
	InsetTheorem();
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::THEOREM_CODE; }
	///
	bool display() const { return true; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
	///
	bool insertInsetAllowed(Inset * inset) const;
};

#endif
