// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright (C) 1998 The LyX Team.
 *
 *======================================================
 */
// The pristine updatable inset: Text


#ifndef INSETERT_H
#define INSETERT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"

class Painter;

/** A colapsable text inset
  
  To write full ert (including styles and other insets) in a given
  space. 
*/
class InsetERT : public InsetText {
public:
    ///
    InsetERT(Buffer *);
    ///
    //     InsetERT(InsetERT const &, Buffer *);
    ///
    ~InsetERT() {}
    ///
    InsetERT * Clone() const;
    ///
    //    void Read(LyXLex &);
    ///
    void Write(ostream &) const;
    ///
    int ascent(Painter &, LyXFont const &) const;
    ///
    int descent(Painter &, LyXFont const &) const;
    ///
    int width(Painter &, LyXFont const & f) const;
    ///
    void draw(Painter & pain, const LyXFont &, int , float &) const;
    ///
    //LString EditMessage() const;
    ///
    void InsetButtonRelease(BufferView *, int, int, int);
    ///
    void InsetButtonPress(BufferView *, int, int, int);
    ///
    void InsetMotionNotify(BufferView *, int, int, int);
    ///
    void Edit(BufferView *, int, int, unsigned int);
    ///
    void InsetUnlock(BufferView *);
    ///
    bool InsertInset(Inset *);
    ///
    void SetFont(LyXFont const &, bool toggleall);

protected:
    ///
    int ascent_closed(Painter &, LyXFont const &) const;
    ///
    int descent_closed(Painter &, LyXFont const &) const;
    ///
    int width_closed(Painter &, LyXFont const & f) const;
    ///
    void draw_closed(Painter & pain, const LyXFont &, int , float &) const;

private:
    ///
    bool
        closed,
	nomotion;
};

#endif
