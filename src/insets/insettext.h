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

/** A text inset is like a TeX box
  
  To write full text (including styles and other insets) in a given
  space. 
*/
class InsetText : public UpdatableInset {
public:
    ///
    enum { TEXT_TO_INSET_OFFSET = 2 };
    ///
    explicit
    InsetText(Buffer *);
    ///
    InsetText(InsetText const &, Buffer *);
    ///
    ~InsetText();
    ///
    Inset * Clone() const;
    ///
    void clear() const { par->clearContents(); }
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
    void draw(Painter & pain, LyXFont const &, int , float &) const;
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
    int Latex(std::ostream &, bool fragile, bool free_spc) const;
    ///
    int Ascii(std::ostream &) const { return 0; }
    ///
    int Linuxdoc(std::ostream &) const { return 0; }
    ///
    int DocBook(std::ostream &) const { return 0; }
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
    void init(Buffer *, InsetText const * ins = 0);
    ///
    void SetParagraphData(LyXParagraph *);
    ///
    void SetAutoBreakRows(bool);
    ///
    void SetDrawLockedFrame(bool);
    ///
    void SetFrameColor(LColor::color);
    ///
    void computeTextRows(Painter &) const;

    LyXParagraph * par;

protected:
    ///
    void UpdateLocal(BufferView *, bool);
    ///
    void WriteParagraphData(std::ostream &) const;
    ///
    void resetPos(Painter &) const;
    ///
    void drawSelection(Painter &, int pos, int baseline, float x);
    ///
    void SingleHeight(Painter &, LyXParagraph * par,int pos,
		      int & asc, int & desc) const;
    ///
    int SingleWidth(Painter &, LyXParagraph * par, int pos) const;
    ///
    LyXFont GetFont(LyXParagraph * par, int pos) const;
    ///
    virtual LyXFont GetDrawFont(LyXParagraph * par, int pos) const;
    ///
    virtual int getMaxTextWidth(Painter &, UpdatableInset const *) const;

    Buffer * buffer;
    ///
    LyXFont current_font;
    ///
    LyXFont real_current_font;
    ///
    mutable int maxAscent;
    ///
    mutable int maxDescent;
    ///
    mutable int insetWidth;
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
    void drawRowSelection(Painter &, int startpos, int endpos, int row,
			  int baseline, float x) const;
    ///
    void drawRowText(Painter &, int startpos, int endpos, int baseline,
                     float x) const;
    ///
    void computeBaselines(int) const;
    ///
    int BeginningOfMainBody(LyXParagraph * par) const;
    ///
    void ShowInsetCursor(BufferView *);
    ///
    void HideInsetCursor(BufferView *);
    ///
    void setPos(Painter &, int x, int y) const;
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
    bool cutSelection();
	///
    bool copySelection();
	///
    bool pasteSelection();
    ///
    bool hasSelection() const
		{ return (selection_start_cursor != selection_end_cursor); }
    ///
    void SetCharFont(int pos, LyXFont const & font);
    ///
    string getText(int);
    ///
    bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
			       int button = 0);
	
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
    LyXCursor selection_start_cursor;
    ///
    LyXCursor selection_end_cursor;
    ///
    mutable LyXCursor cursor;
    ///
    mutable LyXCursor old_cursor;
    ///
    mutable int actrow;
    ///
    bool no_selection;
    ///
    mutable float xpos;
    ///
    mutable bool init_inset;
    ///
    UpdatableInset * the_locking_inset;
    ///
    struct row_struct {
	///
	int asc;
	///
	int desc;
	///
	int pos;
	///
	int baseline;
    };
    ///
    typedef std::vector<row_struct> RowList;
    ///
    mutable RowList rows;
	///
    InsetText & operator = (InsetText const & it) {
	par = it.par;
	buffer = it.buffer; // suspect
	current_font = it.current_font;
	real_current_font = it.real_current_font;
	maxAscent = it.maxAscent;
	maxDescent = it.maxDescent;
	insetWidth = it.insetWidth;
	inset_pos = it.inset_pos;
	inset_x = it.inset_x;
	inset_y = it.inset_y;
	interline_space = it.interline_space;
	selection_start_cursor = selection_end_cursor = cursor = it.cursor;
	actrow = it.actrow;
	no_selection = it.no_selection;
	the_locking_inset = it.the_locking_inset; // suspect
	rows = it.rows;
	return * this;
    }
};
#endif
