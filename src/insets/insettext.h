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


#ifndef INSETTEXT_H
#define INSETTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"
#include "lyxcursor.h"

class Painter;
class BufferView;
class Buffer;
class LyXCursor;
class LyXParagraph;
class LColor;
class LyXText;

/**
 * A text inset is like a TeX box to write full text
 * (including styles and other insets) in a given space. 
 */
class InsetText : public UpdatableInset {
public:
    ///
    enum { TEXT_TO_INSET_OFFSET = 2 };
    ///
    explicit
    InsetText();
    ///
    InsetText(InsetText const &);
    ///
    ~InsetText();
    ///
    Inset * Clone() const;
    ///
    InsetText & operator= (InsetText const & it);
    ///
    void clear() const { par->clearContents(); }
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
    void draw(Painter & pain, LyXFont const &, int , float &) const;
    ///
    void update(BufferView *, LyXFont const &) const;
    ///
    char const * EditMessage() const;
    ///
    void Edit(BufferView *, int, int, unsigned int);
    ///
    void InsetUnlock(BufferView *);
    ///
    bool LockInsetInInset(BufferView *, UpdatableInset *);
    ///
    bool UnlockInsetInInset(BufferView *, UpdatableInset *, bool lr = false);
    ///
    bool UpdateInsetInInset(BufferView *, Inset *);
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
    Inset::Code LyxCode() const { return Inset::TEXT_CODE; }
    ///
    void GetCursorPos(int & x, int & y) const;
    ///
    int InsetInInsetY();
    ///
    void ToggleInsetCursor(BufferView *);
    ///
    bool InsertInset(BufferView *, Inset *);
    ///
    UpdatableInset * GetLockingInset();
    ///
    UpdatableInset * GetFirstLockingInsetOfType(Inset::Code);
    ///
    void SetFont(BufferView *, LyXFont const &, bool toggleall = false);
    ///
    void init(InsetText const * ins = 0);
    ///
    void SetParagraphData(LyXParagraph *);
    ///
    void SetAutoBreakRows(bool);
    ///
    void SetDrawLockedFrame(bool);
    ///
    void SetFrameColor(LColor::color);
    ///
    LyXFont GetDrawFont(Buffer const *, LyXParagraph *, int pos) const;
    ///
    LyXParagraph * par;
    ///

protected:
    ///
    void UpdateLocal(BufferView *, bool what, bool mark_dirty);
    ///
    void WriteParagraphData(Buffer const *, std::ostream &) const;
    ///
    virtual int getMaxTextWidth(Painter &, UpdatableInset const *) const;

    LyXText * text;
    ///
    mutable int drawTextXOffset;
    mutable int drawTextYOffset;
    ///
    bool autoBreakRows;
    bool drawLockedFrame;
    ///
    LColor::color frame_color;

private:
    ///
    int BeginningOfMainBody(Buffer const *, LyXParagraph * par) const;
    ///
    void ShowInsetCursor(BufferView *);
    ///
    void HideInsetCursor(BufferView *);
    ///
    UpdatableInset::RESULT moveRight(BufferView *, bool activate_inset = true);
    ///
    UpdatableInset::RESULT moveLeft(BufferView *, bool activate_inset = true);
    ///
    UpdatableInset::RESULT moveUp(BufferView *);
    ///
    UpdatableInset::RESULT moveDown(BufferView *);
    ///
    bool Delete();
    ///
    void SetCharFont(Buffer const *, int pos, LyXFont const & font);
    ///
    string getText(int);
    ///
    bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
			       int button = 0);
    int cx() const;
    int cy() const;
    int cpos() const;
    LyXParagraph * cpar() const;
    Row * crow() const;
	
    /* Private structures and variables */
    ///
    bool locked;
    ///
    int inset_pos;
    ///
    mutable int inset_x;
    ///
    mutable int inset_y;
    ///
    int interline_space;
    ///
    bool no_selection;
    ///
    mutable float xpos;
    ///
    mutable bool init_inset;
    ///
    UpdatableInset * the_locking_inset;
    ///
    LyXParagraph * old_par;
};
#endif
