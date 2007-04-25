// -*- C++ -*-
/**
 * \file InsetTabular.h
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

#include "Inset.h"
#include "tabular.h"
#include "MailInset.h"

namespace lyx {

class FuncStatus;
class LyXLex;
class BufferView;
class Buffer;
class BufferParams;
class Paragraph;
class CursorSlice;

namespace frontend { class Painter; }

class InsetTabular : public InsetOld {
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
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	///
	virtual docstring const editMessage() const;
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool insetAllowed(InsetBase::Code) const { return true; }
	///
	bool allowSpellCheck() const { return true; }
	///
	bool canTrackChanges() const { return true; }
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool display() const { return tabular.isLongTabular(); }
	///
	int latex(Buffer const &, odocstream &,
	          OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
	              OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
	            OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Code lyxCode() const { return InsetBase::TABULAR_CODE; }
	/// get offset of this cursor slice relative to our upper left corner
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	bool tabularFeatures(LCursor & cur, std::string const & what);
	///
	void tabularFeatures(LCursor & cur, LyXTabular::Feature feature,
			     std::string const & val = std::string());
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	/// number of cells
	size_t nargs() const { return tabular.getNumberOfCells(); }
	///
	boost::shared_ptr<InsetText const> cell(idx_type) const;
	///
	boost::shared_ptr<InsetText> cell(idx_type);
	///
	LyXText * getText(int) const;

	/// set the change for the entire inset
	void setChange(Change const & change);
	/// accept the changes within the inset
	void acceptChanges(BufferParams const & bparams);
	/// reject the changes within the inset
	void rejectChanges(BufferParams const & bparams);

	// this should return true if we have a "normal" cell, otherwise false.
	// "normal" means without width set!
	/// should all paragraphs be output with "Standard" layout?
	bool forceDefaultParagraphs(idx_type cell = 0) const;

	///
	void addPreview(graphics::PreviewLoader &) const;

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
	///
	InsetTabular(InsetTabular const &);
	///
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
	///
	bool getStatus(LCursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	int scroll() const { return scx_; }

private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	void drawCellLines(frontend::Painter &, int x, int y, row_type row,
			   idx_type cell, bool erased) const;
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
	bool copySelection(LCursor & cur);
	///
	bool pasteClipboard(LCursor & cur);
	///
	void cutSelection(LCursor & cur);
	///
	bool isRightToLeft(LCursor & cur) const;
	///
	void getSelection(LCursor & cur, row_type & rs, row_type & re,
			  col_type & cs, col_type & ce) const;
	///
	bool insertPlaintextString(BufferView &, docstring const & buf, bool usePaste);
	/// are we operating on several cells?
	bool tablemode(LCursor & cur) const;

	/// return the "Manhattan distance" to nearest corner
	int dist(BufferView &, idx_type cell, int x, int y) const;
	/// return the cell nearest to x, y
	idx_type getNearestCell(BufferView &, int x, int y) const;

	///
	Buffer const * buffer_;
	///
	mutable idx_type first_visible_cell;
	///
	mutable int scx_;
	/// Ugly boolean used when this inset is dissolved and
	/// InsetTabularMailer should not be used.
	bool is_deleted_;
};


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

} // namespace lyx

#endif
