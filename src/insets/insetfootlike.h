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


#ifndef INSETFOOTLIKE_H
#define INSETFOOTLIKE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

// To have this class is probably a bit overkill... (Lgb)

/** The footnote inset

*/
class InsetFootlike : public InsetCollapsable {
public:
	///
	InsetFootlike(BufferParams const &);
	///
	InsetFootlike(InsetFootlike const &, bool same_id = false);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	bool insetAllowed(Inset::Code) const;
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
};

#endif
