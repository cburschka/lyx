// -*- C++ -*-
/**
 * \file insettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETTEXT_H
#define INSETTEXT_H

#include "updatableinset.h"
#include "RowList_fwd.h"
#include "lyxfont.h"
#include "lyxtext.h"

#include "support/types.h"

#include "frontends/mouse_state.h"

class Buffer;
class BufferParams;
class BufferView;
class Dimension;
class LColor_color;
class CursorSlice;
class Painter;
class ParagraphList;
class Row;


/**
 A text inset is like a TeX box to write full text
 (including styles and other insets) in a given space.
 */
class InsetText : public UpdatableInset {
public:
	///
	explicit InsetText(BufferParams const &);
	///
	InsetText();
	/// empty inset to empty par, or just mark as erased
	void clear(bool just_mark_erased);
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw inset selection
	void drawSelection(PainterInfo & pi, int x, int y) const;
	///
	std::string const editMessage() const;
	///
	bool isTextInset() const { return true; }
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

	/// return x,y of given position relative to the inset's baseline
	void getCursorPos(CursorSlice const & sl, int & x, int & y) const;
	///
	Code lyxCode() const { return TEXT_CODE; }
	///
	void setFont(BufferView *, LyXFont const &,
		     bool toggleall = false,
		     bool selectall = false);
	///
	void setText(std::string const &, LyXFont const &);
	///
	void setAutoBreakRows(bool);
	///
	bool getAutoBreakRows() const { return autoBreakRows_; }
	///
	void setDrawFrame(bool);
	///
	LColor_color frameColor() const;
	///
	void setFrameColor(LColor_color);
	///
	void setViewCache(BufferView const * bv) const;
	///
	bool showInsetDialog(BufferView *) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	LyXText * getText(int) const;
	///
	bool getStatus(LCursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	/// mark as erased for change tracking
	void markErased() { clear(true); }
	/**
	 * Mark as new. Used when pasting in tabular, and adding rows
	 * or columns. Note that pasting will ensure that tracking already
	 * happens, and this just resets the changes for the copied text,
	 * whereas for row/col add, we need to start tracking changes
	 * for the (empty) paragraph contained.
	 */
	void markNew(bool track_changes = false);

	/// append text onto the existing text
	void appendParagraphs(Buffer * bp, ParagraphList &);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	///
	void edit(LCursor & cur, bool left);
	///
	InsetBase * editXY(LCursor & cur, int x, int y) const;

	/// number of cells in this inset
	size_t nargs() const { return 1; }
	///
	ParagraphList & paragraphs();
	///
	ParagraphList const & paragraphs() const;
	///
	bool insetAllowed(Code) const { return true; }
	///
	bool allowSpellCheck() const { return true; }

protected:
	InsetText(InsetText const &);
	///
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	void updateLocal(LCursor &);
	///
	void init();
	///
	void setCharFont(Buffer const &, int pos, LyXFont const & font);
	///
	void removeNewlines();
	///
	void drawFrame(Painter &, int x, int y) const;
	///
	void clearInset(Painter &, int x, int y) const;

	///
	bool autoBreakRows_;
	///
	bool drawFrame_;
	/** We store the LColor::color value as an int to get LColor.h out
	 *  of the header file.
	 */
	int frame_color_;
	///
	mutable lyx::pit_type old_pit;
public:
	///
	mutable LyXText text_;
	///
	mutable LyXFont font_;
};
#endif
