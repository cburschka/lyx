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

using std::ostream;

class Painter;

/** The footnote inset
  
*/
class InsetFoot : public InsetCollapsable {
public:
    ///
    InsetFoot(Buffer *);
    ///
    ~InsetFoot() {}
    ///
    Inset * Clone() const;
    ///
    Inset::Code LyxCode() const { return Inset::FOOT_CODE; }
    ///
    int Latex(ostream &, signed char, bool fp) const;
    ///
    void Write(ostream &) const;
    ///
    void Read(LyXLex &);
    ///
    const char * EditMessage() const;
    ///
    bool InsertInset(BufferView *, Inset * inset);
};

#endif
