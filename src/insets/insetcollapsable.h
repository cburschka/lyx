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


#ifndef INSETCOLLAPSABLE_H
#define INSETCOLLAPSABLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"
#include "lyxfont.h"
#include "LColor.h"


class Painter;

/** A colapsable text inset
  
*/
class InsetCollapsable : public InsetText {
public:
    ///
    static int const TEXT_TO_TOP_OFFSET = 2;
    ///
    static int const TEXT_TO_BOTTOM_OFFSET = 2;
    ///
    explicit
    InsetCollapsable();
    ///
    ~InsetCollapsable() {}
    ///
    Inset * Clone() const;
    ///
    void Read(Buffer const *, LyXLex &);
    ///
    void Write(Buffer const *, std::ostream &) const;
    ///
    int ascent(Painter &, LyXFont const &) const;
    ///
    int descent(Painter &, LyXFont const &) const;
    ///
    int width(Painter &, LyXFont const & f) const;
    ///
    void draw(BufferView *, const LyXFont &, int , float &, bool) const;
    ///
    void update(BufferView *, LyXFont const &, bool =false); 
    ///
    void Edit(BufferView *, int, int, unsigned int);
    ///
    EDITABLE Editable() const;
    ///
    void InsetUnlock(BufferView *);
    ///
    bool display() const { return (!collapsed); }
    ///
    void InsetButtonRelease(BufferView *, int, int, int);
    ///
    void InsetButtonPress(BufferView *, int, int, int);
    ///
    void InsetMotionNotify(BufferView *, int, int, int);
    ///
    void setLabel(string const & l) { label = l; }
    ///
    void setLabelFont(LyXFont & f) { labelfont = f; }
    ///
    void setAutoCollapse(bool f) { autocollapse = f; }
    ///
    int getMaxWidth(Painter & pain, UpdatableInset const *) const;

protected:
    ///
    int ascent_collapsed(Painter &, LyXFont const &) const;
    ///
    int descent_collapsed(Painter &, LyXFont const &) const;
    ///
    int width_collapsed(Painter &, LyXFont const & f) const;
    ///
    void draw_collapsed(Painter & pain, const LyXFont &, int , float &) const;
    ///
    int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
    
    ///
    bool collapsed;
    ///
    LColor::color framecolor;
    ///
    LyXFont labelfont;

private:
    ///
    string label;
    ///
    bool autocollapse;
    ///
    mutable int
	button_length, button_top_y, button_bottom_y;
    ///
    int widthOffset;
};

#endif
