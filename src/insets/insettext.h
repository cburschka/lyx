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
#include "lyxparagraph.h"
#include "LString.h"
//#include "buffer.h"

using std::ostream;
using std::vector;

class Painter;
class BufferView;
class Buffer;

/** A text inset is like a TeX box
  
  To write full text (including styles and other insets) in a given
  space. 
*/
class InsetText : public UpdatableInset {
public:
	///
	enum { TEXT_TO_INSET_OFFSET = 1 };
    ///
    InsetText(Buffer *);
    ///
    InsetText(InsetText const &, Buffer *);
    ///
    ~InsetText();
    ///
    Inset * Clone() const;
    ///
    void Read(LyXLex &);
    ///
    void Write(ostream &) const;
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
    bool UnlockInsetInInset(BufferView *, Inset *, bool lr = false);
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
    int Latex(ostream &, signed char, bool free_spc) const;
    ///
    int Linuxdoc(ostream &) const { return 0; }
    ///
    int DocBook(ostream &) const { return 0; }
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
    void SetFont(BufferView *, LyXFont const &, bool toggleall = false);

    LyXParagraph * par;

protected:
    ///
    void UpdateLocal(BufferView *, bool);
    ///
    void WriteParagraphData(ostream &) const;
    ///
    void resetPos(BufferView *);
    ///
    void drawSelection(Painter &, int pos, int baseline, float x);
    ///
    void SingleHeight(Painter &, LyXParagraph * par,int pos,
		      int & asc, int & desc) const;
    ///
    int SingleWidth(Painter &, LyXParagraph * par, int pos) const;
    ///
    LyXFont GetFont(LyXParagraph * par, int pos) const;

    Buffer * buffer;
    ///
    LyXFont current_font;
    ///
    LyXFont real_current_font;
    ///
    mutable int maxWidth;
    ///
    mutable int maxAscent;
    ///
    mutable int maxDescent;
    ///
    mutable int insetWidth;
    ///
    int widthOffset;
    ///
    bool autoBreakRows;

private:
    ///
    void drawRowSelection(Painter &, int startpos, int endpos, int row,
			  int baseline, float x) const;
    ///
    void drawRowText(Painter &, int startpos, int endpos, int baseline,
                     float x) const;
    ///
    void computeTextRows(Painter &, float x = 0.0) const;
    ///
    void computeBaselines(int) const;
    ///
    int BeginningOfMainBody(LyXParagraph * par) const;
    ///
    void ShowInsetCursor(BufferView *);
    ///
    void HideInsetCursor(BufferView *);
    ///
    void setPos(BufferView *, int x, int y, bool activate_inset = true);
    ///
    bool moveRight(BufferView *, bool activate_inset = true);
    bool moveLeft(BufferView *, bool activate_inset = true);
    bool moveUp(BufferView *, bool activate_inset = true);
    bool moveDown(BufferView *, bool activate_inset = true);
    bool Delete();
    bool cutSelection();
    bool copySelection();
    bool pasteSelection();
    ///
    bool hasSelection() const { return selection_start != selection_end; }
    ///
    void SetCharFont(int pos, LyXFont const & font);
    ///
    string getText(int);
	
    /* Private structures and variables */
    ///
    int inset_pos;
    ///
    int inset_x;
    ///
    int inset_y;
    ///
    int interline_space;
    ///
    int selection_start;
    ///
    int selection_end;
    ///
    int old_x;
    ///
    int cx;
    ///
    int cy;
    ///
    int actpos;
    ///
    int actrow;
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
    typedef vector<row_struct> RowList;
    ///
    mutable RowList rows;
    InsetText & operator = (InsetText const & it) {
	par = it.par;
	buffer = it.buffer; // suspect
	current_font = it.current_font;
	real_current_font = it.real_current_font;
	maxWidth = it.maxWidth;
	maxAscent = it.maxAscent;
	maxDescent = it.maxDescent;
	insetWidth = it.insetWidth;
	inset_pos = it.inset_pos;
	inset_x = it.inset_x;
	inset_y = it.inset_y;
	interline_space = it.interline_space;
	selection_start = it.selection_start;
	selection_end = it.selection_end;
	old_x = it.old_x;
	cx = it.cx;
	cy = it.cy;
	actpos = it.actpos;
	actrow = it.actrow;
	no_selection = it.no_selection;
	the_locking_inset = it.the_locking_inset; // suspect
	rows = it.rows;
	return * this;
    }
};
#endif
