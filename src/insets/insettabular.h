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
class Paragraph;

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
	InsetTabular(InsetTabular const &, Buffer const &, bool same_id = false);
	///
	~InsetTabular();
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	void read(Buffer const *, LyXLex &);
	///
	void write(Buffer const *, std::ostream &) const;
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
	string const editMessage() const;
	///
	void edit(BufferView *, int x, int y, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	bool doClearArea() const;
	///
	void insetUnlock(BufferView *);
	///
	void updateLocal(BufferView *, UpdateCodes, bool mark_dirty) const;
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	bool updateInsetInInset(BufferView *, Inset *);
	///
	unsigned int insetInInsetY();
	///
	UpdatableInset * getLockingInset() const;
	///
	UpdatableInset * getFirstLockingInsetOfType(Inset::Code);
	///
	bool insertInset(BufferView *, Inset *);
	///
	bool insetAllowed(Inset::Code code) const;
	///
	bool isTextInset() const { return true; }
	///
	bool display() const { return tabular->IsLongTabular(); }
	///
	void insetButtonRelease(BufferView *, int, int, int);
	///
	void insetButtonPress(BufferView *, int, int, int);
	///
	void insetMotionNotify(BufferView *, int, int, int);
	///
	void insetKeyPress(XKeyEvent *);
	///
	UpdatableInset::RESULT localDispatch(BufferView *, kb_action,
					     string const &);
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const { return Inset::TABULAR_CODE; }
	///
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	void toggleInsetCursor(BufferView *);
	///
	bool tabularFeatures(BufferView * bv, string const & what);
	///
	void tabularFeatures(BufferView * bv, LyXTabular::Feature feature,
			     string const & val = string());
	///
	int getActCell() const { return actcell; }
	///
	void setFont(BufferView *, LyXFont const &, bool toggleall = false,
	             bool selectall = false);
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	Buffer * bufferOwner() const { return const_cast<Buffer *>(buffer); }
	///
	LyXText * getLyXText(BufferView const *,
			     bool const recursive = false) const;
	///
	void deleteLyXText(BufferView *, bool recursive = true) const;
	///
	void resizeLyXText(BufferView *, bool force = false) const;
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	func_status::value_type getStatus(string const & argument) const;
	///
	std::vector<string> const getLabelList() const;
	///
	void nodraw(bool b) const {
		UpdatableInset::nodraw(b);
	}
	bool nodraw() const;
	///
	int scroll(bool recursive=true) const;
	///
	void scroll(BufferView *bv, float sx) const {
		UpdatableInset::scroll(bv, sx);
	}
	///
	void scroll(BufferView *bv, int offset) const {
		UpdatableInset::scroll(bv, offset);
	}
	///
	Paragraph * getParFromID(int id) const;
	///
	Inset * getInsetFromID(int id) const;
	///
	Paragraph * firstParagraph() const;
	///
	Paragraph * getFirstParagraph(int) const;
	///
	LyXCursor const & cursor(BufferView *) const;
	///
	string const selectNextWord(BufferView *, float & value) const;
	void selectSelectedWord(BufferView *);
	void toggleSelection(BufferView *, bool kill_selection);
	///
	bool searchForward(BufferView *, string const &,
	                   bool const & = true, bool const & = false);
	bool searchBackward(BufferView *, string const &,
	                    bool const & = true, bool const & = false);

	//
	// Public structures and variables
	///
	boost::scoped_ptr<LyXTabular> tabular;
	///
	SigC::Signal0<void> hideDialog;

private:
	///
	bool calculate_dimensions_of_cells(BufferView *, LyXFont const &,
					   bool = false) const;
	///
	void drawCellLines(Painter &, int x, int baseline,
			   int row, int cell) const;
	///
	void drawCellSelection(Painter &, int x, int baseline,
			       int row, int column, int cell) const;
	///
	void showInsetCursor(BufferView *, bool show=true);
	///
	void hideInsetCursor(BufferView *);
	///
	void fitInsetCursor(BufferView *) const;
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
	bool deletable();
	///
	int getCellXPos(int cell) const;
	///
	void resetPos(BufferView *) const;
	///
	void removeTabularRow();
	///
	bool hasSelection() const {
		return sel_cell_start != sel_cell_end;
	}
	///
	void clearSelection() const {
	sel_cell_start = sel_cell_end = 0;
	}
	///
	bool activateCellInset(BufferView *, int x = 0, int y = 0,
			       int button = 0,
			       bool behind = false);
	///
	bool activateCellInsetAbs(BufferView *, int x = 0, int y = 0,
				  int button = 0);
	///
	bool insetHit(BufferView * bv, int x, int y) const;
	///
	int getMaxWidthOfCell(BufferView * bv, int cell) const;
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
	void getSelection(int & scol, int & ecol,
			  int & srow, int & erow) const;
	///
	string selectNextWordInt(BufferView *, float & value) const;

	//
	// Private structures and variables
	///
	InsetText * the_locking_inset;
	///
	InsetText * old_locking_inset;
	///
	Buffer const * buffer;
	///
	mutable LyXCursor cursor_;
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
	///
	bool in_update;
};
#endif
