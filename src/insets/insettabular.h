// -*- C++ -*-
/**
 * \file insettabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
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

#include "inset.h"
#include "tabular.h"
#include "LString.h"
#include "FuncStatus.h"
#include "frontends/mouse_state.h"

class LyXLex;
class Painter;
class BufferView;
class Buffer;
class BufferParams;
class Paragraph;

class InsetTabular : public UpdatableInset {
public:
	///
	enum UpdateCodes {
		NONE = 0,
		CURSOR = 1,
		CELL = 2,
		FULL = 4,
		INIT = 5
	};
	///
	InsetTabular(Buffer const &, int rows = 1, int columns = 1);
	///
	InsetTabular(InsetTabular const &);
	///
	~InsetTabular();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void read(Buffer const *, LyXLex &);
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	string const editMessage() const;
	//
	void insetUnlock(BufferView *);
	///
	void updateLocal(BufferView *, UpdateCodes) const;
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	bool updateInsetInInset(BufferView *, Inset *);
	///
	int insetInInsetY() const;
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
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool display() const { return tabular.isLongTabular(); }
	///
	RESULT localDispatch(FuncRequest const &);
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const { return Inset::TABULAR_CODE; }
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const;
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
	FuncStatus getStatus(string const & argument) const;
	///
	void getLabelList(std::vector<string> &) const;
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
	Inset * getInsetFromID(int id) const;
	///
	ParagraphList * getParagraphs(int) const;
	///
	LyXCursor const & cursor(BufferView *) const;
	///
	bool allowSpellcheck() const { return true; }
	///
	WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float & value) const;
	///
	void selectSelectedWord(BufferView *);
	///
	void toggleSelection(BufferView *, bool kill_selection);

	void markErased();

	/// find next change
	bool nextChange(BufferView *, lyx::pos_type & length);
	///
	bool searchForward(BufferView *, string const &,
			   bool = true, bool = false);
	bool searchBackward(BufferView *, string const &,
			    bool = true, bool = false);

	// this should return true if we have a "normal" cell, otherwise true.
	// "normal" means without width set!
	bool forceDefaultParagraphs(Inset const * in) const;

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	//
	// Public structures and variables
	///
	mutable LyXTabular tabular;

	/// are some cells selected ?
	bool hasSelection() const {
		return has_selection;
	}

	///
	virtual BufferView * view() const;
	///
	Buffer const * buffer() const;

	/// set the owning buffer
	void buffer(Buffer * b);

private:
	///
	void lfunMousePress(FuncRequest const &);
	///
	// the bool return is used to see if we opened a dialog so that we can
	// check this from an outer inset and open the dialog of the outer inset
	// if that one has one!
	///
	bool lfunMouseRelease(FuncRequest const &);
	///
	void lfunMouseMotion(FuncRequest const &);
	///
	void calculate_dimensions_of_cells(MetricsInfo & mi) const;
	///
	void drawCellLines(Painter &, int x, int baseline,
			   int row, int cell) const;
	///
	void drawCellSelection(Painter &, int x, int baseline,
			       int row, int column, int cell) const;
	///
	void fitInsetCursor(BufferView *) const;
	///
	void setPos(BufferView *, int x, int y) const;
	///
	RESULT moveRight(BufferView *, bool lock = true);
	///
	RESULT moveLeft(BufferView *, bool lock = true);
	///
	RESULT moveUp(BufferView *, bool lock = true);
	///
	RESULT moveDown(BufferView *, bool lock = true);
	///
	bool moveNextCell(BufferView *, bool lock = false);
	///
	bool movePrevCell(BufferView *, bool lock = false);
	///
	int getCellXPos(int cell) const;
	///
	void resetPos(BufferView *) const;
	///
	void removeTabularRow();
	///
	void clearSelection() const {
		sel_cell_start = sel_cell_end = 0;
		has_selection = false;
	}
	void setSelection(int start, int end) const {
		sel_cell_start = start;
		sel_cell_end = end;
		has_selection = true;
	}
	///
	bool activateCellInset(BufferView *, int x = 0, int y = 0,
			       mouse_button::state button = mouse_button::none,
			       bool behind = false);
	///
	bool activateCellInsetAbs(BufferView *, int x = 0, int y = 0,
				  mouse_button::state button = mouse_button::none);
	///
	bool insetHit(BufferView * bv, int x, int y) const;
	///
	bool hasPasteBuffer() const;
	///
	bool copySelection(BufferView *);
	///
	bool pasteSelection(BufferView *);
	///
	bool cutSelection(BufferParams const & bp);
	///
	bool isRightToLeft(BufferView *);
	///
	void getSelection(int & scol, int & ecol,
			  int & srow, int & erow) const;
	///
	WordLangTuple selectNextWordInt(BufferView *, float & value) const;
	///
	bool insertAsciiString(BufferView *, string const & buf, bool usePaste);

	//
	// Private structures and variables
	///
	InsetText * the_locking_inset;
	///
	InsetText * old_locking_inset;
	///
	Buffer const * buffer_;
	///
	mutable int cursorx_;
	///
	mutable int cursory_;
	///
	mutable unsigned int inset_x;
	///
	mutable unsigned int inset_y;
	/// true if a set of cells are selected
	mutable bool has_selection;
	/// the starting cell selection nr
	mutable int sel_cell_start;
	/// the ending cell selection nr
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
	mutable int in_reset_pos;
};


#include "mailinset.h"


class InsetTabularMailer : public MailInset {
public:
	///
	InsetTabularMailer(InsetTabular const & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string(Buffer const &) const;
	/// Returns the active cell if successful, else -1.
	static int string2params(string const &, InsetTabular &);
	///
	static string const params2string(InsetTabular const &);
private:
	///
	static string const name_;
	///
	InsetTabular & inset_;
};

string const featureAsString(LyXTabular::Feature feature);

#endif
