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


#ifndef INSETFOOT_H
#define INSETFOOT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

class Painter;

/** The footnote inset
  
*/
class InsetFoot : public InsetCollapsable {
public:
    ///
	explicit
    InsetFoot(Buffer *);
    ///
    ~InsetFoot() {}
    ///
    Inset * Clone() const;
    ///
    Inset::Code LyxCode() const { return Inset::FOOT_CODE; }
    ///
    int Latex(std::ostream &, signed char, bool fp) const;
    ///
    void Write(std::ostream &) const;
    ///
    void Read(LyXLex &);
    ///
    const char * EditMessage() const;
    ///
    bool InsertInset(BufferView *, Inset * inset);
    ///
    bool InsertInsetAllowed(Inset * inset) const;
    ///
    LyXFont GetDrawFont(LyXParagraph * par, int pos) const;
    ///
};

#endif
