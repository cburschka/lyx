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
	void write(Buffer const * buf, std::ostream & os) const;
	///
	Inset::Code lyxCode() const { return Inset::FOOT_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
};

#endif
