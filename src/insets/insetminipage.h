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

class Painter;

/** The footnote inset
  
*/
class InsetMinipage : public InsetCollapsable {
public:
	///
	explicit
	InsetMinipage();
	///
	~InsetMinipage() {}
	///
	void Write(Buffer const * buf, ostream & os) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::MINIPAGE_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	const char * EditMessage() const;
	///
	bool InsertInset(BufferView *, Inset * inset);
	///
	bool InsertInsetAllowed(Inset * inset) const;
	///
	LyXFont GetDrawFont(BufferView *, LyXParagraph * par, int pos) const;
};

#endif
