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

#ifndef InsetMinipage_H
#define InsetMinipage_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** The minipage inset
  
*/
class InsetMinipage : public InsetCollapsable {
public:
	///
	InsetMinipage();
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MINIPAGE_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
};

#endif
