// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
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

#include <sigc++/signal_system.h>
#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include "tabular.h"
#include "LString.h"
#include "lyxcursor.h"
#include "func_status.h"

class LyXLex;
class Painter;
class BufferView;
class Buffer;

class InsetTabular : public UpdatableInset {
public:
	///
	enum UpdateCodes {
		NONE = 0,
		CURSOR = 1,
		CELL = 2,
		SELECTION = 3,
		FULL = 4,
		INIT = 5
	};
	///
	InsetTabular(Buffer const &, int rows = 1, int columns = 1);
	///
	InsetTabular(InsetTabular const &, Buffer const &);
	///
	~InsetTabular();
	///
	Inset * Clone(Buffer const &) const;
	///
	void Read(Buffer const *, LyXLex &);
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const & f) const;
	///
	void draw(BufferView *, const LyXFont &, int , float &, bool) const;
	///
	void update(BufferView *, LyXFont const &, bool = false);
	///
	string const EditMessage() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int);
	///
	bool doClearArea() const;
	///
	void InsetUnlock(BufferView *);
	///
	void UpdateLocal(BufferView *, UpdateCodes, bool mark_dirty) const;
	///
	bool LockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool UnlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	bool UpdateInsetInInset(BufferView *, Inset *);
	///
	unsigned int InsetInInsetY();
	///
	UpdatableInset * GetLockingInset();
	///
	UpdatableInset * GetFirstLockingInsetOfType(Inset::Code);
	///
	bool InsertInset(BufferView *, Inset *);
	///
	bool IsTextInset() const { return true; }
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
	UpdatableInset::RESULT LocalDispatch(BufferView *, kb_action,
					     string const &);
	///
	int Latex(Buffer const *, std::ostream &, bool, bool) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures & features) const;
	///
	Inset::Code LyxCode() const { return Inset::TABULAR_CODE; }
	///
	void GetCursorPos(BufferView *, int & x, int & y) const;
	///
	void ToggleInsetCursor(BufferView *);
	///
	bool TabularFeatures(BufferView * bv, string const & what);
	///
	void TabularFeatures(BufferView * bv, LyXTabular::Feature feature,
			     string const & val = string());
	///
	int GetActCell() const { return actcell; }
	///
	void SetFont(BufferView *, LyXFont const &, bool toggleall = false,
	             bool selectall = false);
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	Buffer * BufferOwner() const { return const_cast<Buffer *>(buffer); }
	///
	LyXText * getLyXText(BufferView const *,
			     bool const recursive = false) const;
	///
	void deleteLyXText(BufferView *, bool recursive = true) const;
	///
	void resizeLyXText(BufferView *, bool force = false) const;
	///
	void OpenLayoutDialog(BufferView *) const;
	///
	bool ShowInsetDialog(BufferView *) const;
	///
	func_status::value_type getStatus(string const & argument) const;
	///
	std::vector<string> const getLabelList() const;
	///
	void nodraw(bool b) {
		UpdatableInset::nodraw(b);
	}
	bool nodraw() const;
	///
	int scroll(bool recursive=true) const;
	void scroll(BufferView *bv, float sx) const {
		UpdatableInset::scroll(bv, sx);
	}
	void scroll(BufferView *bv, int offset) const {
		UpdatableInset::scroll(bv, offset);
	}

	//
	// Public structures and variables
	///
	//LyXTabular * tabular;
	boost::scoped_ptr<LyXTabular> tabular;
	///
	SigC::Signal0<void> hideDialog;

private:
	///
	bool calculate_dimensions_of_cells(BufferView *, LyXFont const &,
					   bool = false) const;
	///
	void DrawCellLines(Painter &, int x, int baseline,
			   int row, int cell) const;
	///
	void DrawCellSelection(Painter &, int x, int baseline,
			       int row, int column, int cell) const;
	///
	void ShowInsetCursor(BufferView *, bool show=true);
	///
	void HideInsetCursor(BufferView *);
	///
	void setPos(BufferView *, int x, int y) const;
	///
	UpdatableInset::RESULT moveRight(BufferView *, bool lock = true);
	///
	UpdatableInset::RESULT moveLeft(BufferView *, bool lock = true);
	///
	UpdatableInset::RESULT moveUp(BufferView *, bool lock = true);
	///
	UpdatableInset::RESULT moveDown(BufferView *, bool lock = true);
	///
	bool moveNextCell(BufferView *, bool lock = false);
	///
	bool movePrevCell(BufferView *, bool lock = false);
	///
	bool Delete();
	///
	int getCellXPos(int cell) const;
	///
	void resetPos(BufferView *) const;
	///
	void RemoveTabularRow();
	///
	bool hasSelection() const {
		return sel_cell_start != sel_cell_end;
	}
	///
	void clearSelection() const {
	sel_cell_start = sel_cell_end = 0;
	}
	///
	bool ActivateCellInset(BufferView *, int x = 0, int y = 0,
			       int button = 0,
			       bool behind = false);
	///
	bool ActivateCellInsetAbs(BufferView *, int x = 0, int y = 0,
				  int button = 0);
	///
	bool InsetHit(BufferView * bv, int x, int y) const;
	///
	int GetMaxWidthOfCell(BufferView * bv, int cell) const;
	///
	bool hasPasteBuffer() const;
	///
	bool copySelection(BufferView *);
	///
	bool pasteSelection(BufferView *);
	///
	bool cutSelection();
	///
	bool isRightToLeft(BufferView *);
	///
	void getSelection(int & scol, int & ecol, int & srow, int & erow) const;

	//
	// Private structures and variables
	///
	InsetText * the_locking_inset;
	///
	InsetText * old_locking_inset;
	///
	Buffer const * buffer;
	///
	mutable LyXCursor cursor;
	///
	mutable unsigned int inset_x;
	///
	mutable unsigned int inset_y;
	///
	mutable int sel_cell_start;
	///
	mutable int sel_cell_end;
	///
	mutable int actcell;
	///
	mutable int oldcell;
	///
	mutable int actcol;
	///
	mutable int actrow;
	///
	mutable int first_visible_cell;
	///
	bool no_selection;
	///
	mutable bool locked;
	///
	mutable UpdateCodes need_update;
};
#endif
