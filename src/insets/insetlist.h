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

#ifndef InsetList_H
#define InsetList_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** The list inset
  
*/
class InsetList : public InsetCollapsable {
public:
	///
	InsetList();
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::FOOT_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	const char * EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
};

#endif
