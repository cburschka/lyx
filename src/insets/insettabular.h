// -*- C++ -*-
/**
 * \file insettabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
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
#include "frontends/mouse_state.h"

class FuncStatus;
class LyXLex;
class Painter;
class BufferView;
class Buffer;
class BufferParams;
class Paragraph;

class InsetTabular : public UpdatableInset {
public:
	///
	InsetTabular(Buffer const &, int rows = 1, int columns = 1);
	///
	InsetTabular(InsetTabular const &);
	///
	~InsetTabular();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	std::string const editMessage() const;
	///
	void updateLocal(BufferView *) const;
	///
	int insetInInsetY() const;
	///
	bool insertInset(BufferView *, InsetOld *);
	///
	bool insetAllowed(InsetOld::Code code) const;
	///
	bool isTextInset() const { return true; }
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool display() const { return tabular.isLongTabular(); }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::TABULAR_CODE; }
	/// get the absolute screen x,y of the cursor
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	bool tabularFeatures(BufferView * bv, std::string const & what);
	///
	void tabularFeatures(BufferView * bv, LyXTabular::Feature feature,
			     std::string const & val = std::string());
	///
	int getActCell() const { return actcell; }
	///
	void setFont(BufferView *, LyXFont const &, bool toggleall = false,
		     bool selectall = false);
	///
	void deleteLyXText(BufferView *) const;
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	FuncStatus getStatus(std::string const & argument) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
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
	ParagraphList * getParagraphs(int) const;
	///
	int numParagraphs() const;
	///
	LyXText * getText(int) const;

	///
	void markErased();

	// this should return true if we have a "normal" cell, otherwise true.
	// "normal" means without width set!
	bool forceDefaultParagraphs(InsetOld const * in) const;

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	//
	// Public structures and variables
	///
	mutable LyXTabular tabular;

	/// are some cells selected ?
	bool hasSelection() const { return has_selection; }

	///
	Buffer const & buffer() const;

	/// set the owning buffer
	void buffer(Buffer * buf);
	/// lock cell with given index
	void edit(BufferView * bv, bool);
	///
	void edit(BufferView * bv, int, int);
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
private:
	///
	void lfunMousePress(FuncRequest const &);
	///
	void lfunMouseRelease(FuncRequest const &);
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
	DispatchResult moveRight(BufferView *, bool lock);
	///
	DispatchResult moveLeft(BufferView *, bool lock);
	///
	DispatchResult moveUp(BufferView *, bool lock);
	///
	DispatchResult moveDown(BufferView *, bool lock);
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
			       bool behind = false);
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
	void getSelection(int & scol, int & ecol, int & srow, int & erow) const;
	///
	bool insertAsciiString(BufferView *, std::string const & buf, bool usePaste);

	//
	// Private structures and variables
	///
	InsetText * the_locking_inset;
	///
	Buffer const * buffer_;
	///
	mutable int cursorx_;
	///
	mutable int cursory_;
	/// true if a set of cells are selected
	mutable bool has_selection;
	/// the starting cell selection nr
	mutable int sel_cell_start;
	/// the ending cell selection nr
	mutable int sel_cell_end;
	///
	mutable int actcell;
	///
	mutable int actcol;
	///
	mutable int actrow;
	///
	mutable int first_visible_cell;
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
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	/// Returns the active cell if successful, else -1.
	static int string2params(std::string const &, InsetTabular &);
	///
	static std::string const params2string(InsetTabular const &);
private:
	///
	static std::string const name_;
	///
	InsetTabular & inset_;
};

std::string const featureAsString(LyXTabular::Feature feature);

#endif
