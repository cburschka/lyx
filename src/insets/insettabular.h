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
class CursorSlice;


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
	void updateLocal(LCursor & cur) const;
	///
	bool insetAllowed(InsetOld::Code) const { return true; }
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
	void getCursorPos(CursorSlice const & cur, int & x, int & y) const;
	///
	bool tabularFeatures(LCursor & cur, std::string const & what);
	///
	void tabularFeatures(LCursor & cur, LyXTabular::Feature feature,
			     std::string const & val = std::string());
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	FuncStatus getStatus(std::string const & argument, int cell) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	int numParagraphs() const;
	///
	LyXText * getText(int) const;

	///
	void markErased();

	// this should return true if we have a "normal" cell, otherwise true.
	// "normal" means without width set!
	bool forceDefaultParagraphs(InsetBase const * in) const;

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	/// are some cells selected ?
	bool hasSelection() const { return has_selection; }
	///
	Buffer const & buffer() const;

	/// set the owning buffer
	void buffer(Buffer * buf);
	/// lock cell with given index
	void edit(LCursor & cur, bool left);
	///
	InsetBase * editXY(LCursor & cur, int x, int y);
	/// can we go further down on mouse click?
	bool descendable() const { return true; }

	//
	// Public structures and variables
	///
	mutable LyXTabular tabular;

protected:
	///
	DispatchResult priv_dispatch(LCursor & cur, FuncRequest const & cmd);
private:
	///
	void lfunMousePress(LCursor & cur, FuncRequest const & cmd);
	///
	void lfunMouseRelease(LCursor & cur, FuncRequest const & cmd);
	///
	void lfunMouseMotion(LCursor & cur, FuncRequest const & cmd);
	///
	void calculate_dimensions_of_cells(MetricsInfo & mi) const;
	///
	void drawCellLines(Painter &, int x, int baseline,
			   int row, int cell) const;
	///
	void drawCellSelection(Painter &, int x, int baseline,
			       int row, int column, int cell) const;
	///
	void setPos(LCursor & cur, int x, int y) const;
	///
	bool moveRight(LCursor & cur);
	///
	bool moveLeft(LCursor & cur);
	///
	bool moveUp(LCursor & cur);
	///
	bool moveDown(LCursor & cur);

	///
	bool moveRightLock(LCursor & cur);
	///
	bool moveLeftLock(LCursor & cur);
	///
	bool moveUpLock(LCursor & cur);
	///
	bool moveDownLock(LCursor & cur);

	///
	bool moveNextCell(LCursor & cur);
	///
	bool movePrevCell(LCursor & cur);


	///
	int getCellXPos(int cell) const;
	///
	void resetPos(LCursor & cur) const;
	///
	void removeTabularRow();
	///
	void clearSelection() const;
	///
	void setSelection(int start, int end) const;
	///
	void activateCellInset(LCursor &, int cell, int x, int y);
	///
	void activateCellInset(LCursor &, int cell, bool behind);
	///
	bool hasPasteBuffer() const;
	///
	bool copySelection(BufferView &);
	///
	bool pasteSelection(BufferView &);
	///
	bool cutSelection(BufferParams const & bp);
	///
	bool isRightToLeft(LCursor & cur);
	///
	void getSelection(int cell,
		int & scol, int & ecol, int & srow, int & erow) const;
	///
	bool insertAsciiString(BufferView &, std::string const & buf, bool usePaste);

	//
	// Private structures and variables
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
	mutable int first_visible_cell;
	///
	mutable int in_reset_pos;
	/// tablemode == true  means we operate on the table as such,
	// i.e. select cells instead of characters in a cell etc.
	// tablemode == false directs most LFUN handling to the 'current' cell.
	mutable bool tablemode;
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
