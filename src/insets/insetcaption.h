// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 *======================================================
 */


#ifndef INSETCAPTION_H
#define INSETCAPTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	void Read(Buffer const * buf, LyXLex & lex);
	///
	Inset::Code LyxCode() const {
		return CAPTION_CODE;
	}
protected:
private:
};

#endif
