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
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::THEOREM_CODE; }
	///
	bool display() const { return true; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
};

#endif
