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

#include "lyxinset.h"
#include "lyxfont.h"
#include "LColor.h"


class Painter;
class InsetText;
class LyXText;

/** A colapsable text inset
  
*/
class InsetCollapsable : public UpdatableInset {
public:
    ///
    enum UpdateCodes {
	NONE = 0,
	FULL
    };
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
    bool IsTextInset() const { return true; }
    ///
    bool doClearArea() const;
    ///
    void InsetUnlock(BufferView *);
    ///
    bool needFullRow() const { return !collapsed; }
    ///
    bool LockInsetInInset(BufferView *, UpdatableInset *);
    ///
    bool UnlockInsetInInset(BufferView *, UpdatableInset *, bool lr = false);
    ///
    bool UpdateInsetInInset(BufferView *, Inset *);
    ///
    int InsetInInsetY();
    ///
    void InsetButtonRelease(BufferView *, int, int, int);
    ///
    void InsetButtonPress(BufferView *, int, int, int);
    ///
    void InsetMotionNotify(BufferView *, int, int, int);
    ///
    void InsetKeyPress(XKeyEvent *);
    ///
    UpdatableInset::RESULT LocalDispatch(BufferView *, int, string const &);
    ///
    int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
    ///
    int Ascii(Buffer const *, std::ostream &) const { return 0; }
    ///
    int Linuxdoc(Buffer const *, std::ostream &) const { return 0; }
    ///
    int DocBook(Buffer const *, std::ostream &) const { return 0; }
    ///
    void Validate(LaTeXFeatures & features) const;
    ///
    void GetCursorPos(BufferView *, int & x, int & y) const;
    ///
    void ToggleInsetCursor(BufferView *);
    ///
    UpdatableInset * GetLockingInset();
    ///
    UpdatableInset * GetFirstLockingInsetOfType(Inset::Code);
    ///
    void SetFont(BufferView *, LyXFont const &, bool toggleall = false);
    ///
    void setLabel(string const & l) { label = l; }
    ///
    void setLabelFont(LyXFont & f) { labelfont = f; }
    ///
    void setAutoCollapse(bool f) { autocollapse = f; }
    ///
    int getMaxWidth(Painter & pain, UpdatableInset const *) const;
    ///
    LyXText * getLyXText(BufferView *) const;
    void deleteLyXText(BufferView *);

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
    ///
    InsetText * inset;
    // Instead of making these ints protected we could have a
    // protected method "clickInButton" (Lgb)
    ///
    mutable int
	button_length, button_top_y, button_bottom_y;

private:
    ///
    string label;
    ///
    bool autocollapse;
    ///
    int widthCollapsed;
    ///
    int oldWidth;
    ///
    mutable int topx;
    mutable int topbaseline;
    mutable UpdateCodes need_update;
};

#endif
