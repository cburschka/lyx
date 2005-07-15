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


// This is Juergen's rewrite of the tabular (table) support.

// Things to think of when designing the new tabular support:
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
//    kind of an inset. [Done]
// o enhance longtable support

// Lgb

#ifndef INSETTABULAR_H
#define INSETTABULAR_H

#include "inset.h"
#include "tabular.h"

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
	InsetTabular(Buffer const &, row_type rows = 1,
	             col_type columns = 1);
	///
	~InsetTabular();
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	///
	std::string const editMessage() const;
	///
	bool insetAllowed(InsetBase::Code) const { return true; }
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
	Code lyxCode() const { return InsetBase::TABULAR_CODE; }
	/// get offset of this cursor slice relative to our upper left corner
	void getCursorPos(CursorSlice const & sl, int & x, int & y) const;
	///
	bool tabularFeatures(LCursor & cur, std::string const & what);
	///
	void tabularFeatures(LCursor & cur, LyXTabular::Feature feature,
			     std::string const & val = std::string());
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	/// number of cells
	size_t nargs() const;
	///
	boost::shared_ptr<InsetText const> cell(idx_type) const;
	///
	boost::shared_ptr<InsetText> cell(idx_type);
	///
	LyXText * getText(int) const;

	///
	void markErased();

	// this should return true if we have a "normal" cell, otherwise true.
	// "normal" means without width set!
	bool forceDefaultParagraphs(InsetBase const * in) const;

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	///
	Buffer const & buffer() const;

	/// set the owning buffer
	void buffer(Buffer const * buf);
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
	InsetTabular(InsetTabular const &);

	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);

	///
	bool getStatus(LCursor & cur, FuncRequest const & cmd, FuncStatus &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	void drawCellLines(Painter &, int x, int y, row_type row,
	                   idx_type cell) const;
	///
	void setCursorFromCoordinates(LCursor & cur, int x, int y) const;

	///
	void moveNextCell(LCursor & cur);
	///
	void movePrevCell(LCursor & cur);
	///
	int getCellXPos(idx_type cell) const;
	///
	void resetPos(LCursor & cur) const;
	///
	void removeTabularRow();
	///
	bool hasPasteBuffer() const;
	///
	bool copySelection(LCursor & cur);
	///
	bool pasteSelection(LCursor & cur);
	///
	void cutSelection(LCursor & cur);
	///
	bool isRightToLeft(LCursor & cur) const;
	///
	void getSelection(LCursor & cur, row_type & rs, row_type & re,
	                  col_type & cs, col_type & ce) const;
	///
	bool insertAsciiString(BufferView &, std::string const & buf, bool usePaste);
	/// are we operating on several cells?
	bool tablemode(LCursor & cur) const;

	/// return the "Manhattan distance" to nearest corner
	int dist(idx_type cell, int x, int y) const;
	/// return the cell nearest to x, y
	idx_type getNearestCell(int x, int y) const;

	///
	Buffer const * buffer_;
	///
	mutable int cursorx_;
	///
	mutable idx_type first_visible_cell;
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
	///
	static void string2params(std::string const &, InsetTabular &);
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
