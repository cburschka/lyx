// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright (C) 1995-2000 The LyX Team.
 *
 *======================================================
 */
// This is the rewrite of the tabular (table) support.

// It will probably be a lot of work.

// One first goal could be to make the inset read the old table format
// and just output it again... no viewing at all.

// When making the internal structure of tabular support I really think
// that STL containers should be used. This will separate the container from
// the rest of the code, which is a good thing.

// Ideally the tabular support should do as the mathed and use
// LaTeX in the .lyx file too.

// Things to think of when desingning the new tabular support:
// - color support (colortbl, color)
// - decimal alignment (dcloumn)
// - custom lines (hhline)
// - rotation
// - multicolumn
// - multirow
// - column styles

// This is what I have written about tabular support in the LyX3-Tasks file:
//
//  o rewrite of table code. Should probably be written as some
//          kind of an inset. At least get the code out of the kernel.
//                - colortbl  -multirow
//                - hhline    -multicolumn
//                - dcolumn
// o enhance longtable support

// Lgb

#ifndef INSETTABULAR_H
#define INSETTABULAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "tabular.h"
#include "LString.h"
#include "lyxcursor.h"

class LyXLex;
class Painter;
class BufferView;
class Buffer;

class InsetTabular : public UpdatableInset {
public:
    ///
    InsetTabular(Buffer *, int rows=1, int columns=1);
    ///
    InsetTabular(InsetTabular const &, Buffer *);
    ///
    ~InsetTabular();
    ///
    Inset * Clone() const;
    ///
    void Read(LyXLex &);
    ///
    void Write(std::ostream &) const;
    ///
    int ascent(Painter &, LyXFont const &) const;
    ///
    int descent(Painter &, LyXFont const &) const;
    ///
    int width(Painter &, LyXFont const & f) const;
    ///
    void draw(Painter & pain, const LyXFont &, int , float &) const;
    ///
    const char * EditMessage() const;
    ///
    void Edit(BufferView *, int x, int y, unsigned int);
    ///
    void InsetUnlock(BufferView *);
    ///
    void UpdateLocal(BufferView *, bool flag = true);
    ///
    bool LockInsetInInset(BufferView *, UpdatableInset *);
    ///
    bool UnlockInsetInInset(BufferView *, UpdatableInset *, bool lr=false);
    ///
    bool UpdateInsetInInset(BufferView *, Inset *);
    ///
    int InsetInInsetY();
    ///
    bool display() const { return tabular->IsLongTabular(); }
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
    int Latex(std::ostream &, bool, bool) const;
    ///
    int Ascii(std::ostream &) const;
    ///
    int Linuxdoc(std::ostream &) const;
    ///
    int DocBook(std::ostream &) const;
    ///
    void Validate(LaTeXFeatures & features) const;
    ///
    Inset::Code LyxCode() const { return Inset::TABULAR_CODE; }
    ///
    void GetCursorPos(int & x, int & y) const;
    ///
    void ToggleInsetCursor(BufferView *);
    ///
    void TabularFeatures(BufferView * bv, int feature, string val="");
    ///
    int GetActCell() { return actcell; }
    ///
    void SetFont(BufferView *, LyXFont const &, bool toggleall = false);
    ///
    /// Public structures and variables
    ///
    LyXTabular * tabular;

private:
    void calculate_width_of_cells(Painter &, LyXFont const &) const;
    ///
    void DrawCellLines(Painter &, int x, int baseline, int row, int cell) const;
    ///
    void ShowInsetCursor(BufferView *);
    ///
    void HideInsetCursor(BufferView *);
    ///
    void setPos(Painter &, int x, int y) const;
    ///
    bool SetCellDimensions(Painter & pain, int cell, int row);
    ///
    UpdatableInset::RESULT moveRight(BufferView *, bool lock=true);
    UpdatableInset::RESULT moveLeft(BufferView *, bool lock=true);
    UpdatableInset::RESULT moveUp();
    UpdatableInset::RESULT moveDown();
    bool moveNextCell();
    bool movePrevCell();
    bool Delete();
    ///
    void resetPos(Painter &) const;
    ///
    void RemoveTabularRow();
    ///
    bool hasCharSelection() const {return (sel_pos_start != sel_pos_end);}
    bool hasCellSelection() const {return hasCharSelection() &&
				 (sel_cell_start != sel_cell_end);}
    ///
    bool ActivateCellInset(BufferView *, int x=0, int y=0, int button=0);
    ///
    bool InsetHit(BufferView * bv, int x, int y) const;

    ///
    /// Private structures and variables
    ///
    InsetText
        * the_locking_inset;
    Buffer
        * buffer;
    mutable LyXCursor
        cursor,
	old_cursor;
    mutable int
        inset_pos,
        inset_x, inset_y,
        sel_pos_start,
	sel_pos_end,
	sel_cell_start,
	sel_cell_end,
        actcell,
        actcol,
        actrow;
    bool
        no_selection;
    mutable bool
        init;
};
#endif
