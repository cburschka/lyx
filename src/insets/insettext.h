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
#include "ParagraphList_fwd.h"
#include "RowList_fwd.h"
#include "lyxtext.h"

#include "support/types.h"

#include "frontends/mouse_state.h"


class Buffer;
class BufferParams;
class BufferView;
class Dimension;
class LColor_color;
class LyXCursor;
class Painter;
class Paragraph;
class Row;

/**
 A text inset is like a TeX box to write full text
 (including styles and other insets) in a given space.
 @author: Jürgen Vigna
 */
class InsetText : public UpdatableInset {
public:
	///
	enum DrawFrame {
		///
		NEVER = 0,
		///
		LOCKED,
		///
		ALWAYS
	};
	///
	explicit InsetText(BufferParams const &);
	///
	InsetText(InsetText const &);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void operator=(InsetText const & it);
	/// empty inset to empty par, or just mark as erased
	void clear(bool just_mark_erased);
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	int textWidth() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
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
		     OutputParams const &) const ;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const ;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::TEXT_CODE; }
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	int insetInInsetY() const;
	///
	bool insertInset(BufferView *, InsetOld *);
	///
	bool insetAllowed(InsetOld::Code) const;
	///
	void setFont(BufferView *, LyXFont const &,
		     bool toggleall = false,
		     bool selectall = false);
	///
	void writeParagraphData(Buffer const &, std::ostream &) const;
	///
	void setText(std::string const &, LyXFont const &);
	///
	void setAutoBreakRows(bool);
	///
	bool getAutoBreakRows() const { return autoBreakRows_; }
	///
	void setDrawFrame(DrawFrame);
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
	int scroll(bool recursive = true) const;
	///
	void scroll(BufferView * bv, float sx) const {
		UpdatableInset::scroll(bv, sx);
	}
	///
	void scroll(BufferView * bv, int offset) const {
		UpdatableInset::scroll(bv, offset);
	}
	///
	void clearSelection(BufferView * bv);
	///
	ParagraphList * getParagraphs(int) const;
	///
	LyXText * getText(int) const;

	/// mark as erased for change tracking
	void markErased() { clear(true); };
	/**
	 * Mark as new. Used when pasting in tabular, and adding rows
	 * or columns. Note that pasting will ensure that tracking already
	 * happens, and this just resets the changes for the copied text,
	 * whereas for row/col add, we need to start tracking changes
	 * for the (empty) paragraph contained.
	 */
	void markNew(bool track_changes = false);

	///
	bool checkInsertChar(LyXFont &);
	///
	void getDrawFont(LyXFont &) const;
	/// append text onto the existing text
	void appendParagraphs(Buffer * bp, ParagraphList &);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	///
	void edit(BufferView *, bool);
	///
	void edit(BufferView *, int, int);

	///
	int numParagraphs() const { return 1; }
	///
	mutable ParagraphList paragraphs;
protected:
	///
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
	///
	void updateLocal(BufferView *, bool mark_dirty);

private:
	///
	void init();
	///
	void lfunMousePress(FuncRequest const &);
	///
	void lfunMouseMotion(FuncRequest const &);
	///
	void lfunMouseRelease(FuncRequest const &);
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	void sanitizeEmptyText(BufferView *);

	///
	DispatchResult moveRight(BufferView *);
	///
	DispatchResult moveLeft(BufferView *);
	///
	DispatchResult moveRightIntern(BufferView *, bool front,
					       bool activate_inset = true,
					       bool selecting = false);
	///
	DispatchResult moveLeftIntern(BufferView *, bool front,
					      bool activate_inset = true,
					      bool selecting = false);

	///
	DispatchResult moveUp(BufferView *);
	///
	DispatchResult moveDown(BufferView *);
	///
	void setCharFont(Buffer const &, int pos, LyXFont const & font);
	///
	bool checkAndActivateInset(BufferView * bv, bool front);
	///
	bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0);
	///
	void removeNewlines();
	///
	int cx() const;
	///
	int cy() const;
	///
	lyx::pos_type cpos() const;
	///
	ParagraphList::iterator cpar() const;
	///
	RowList::iterator crow() const;
	///
	void drawFrame(Painter &, int x) const;
	///
	void clearInset(BufferView *, int start_x, int baseline) const;
	///
	void collapseParagraphs(BufferView *);

	/* Private structures and variables */
	///
	bool autoBreakRows_;
	///
	DrawFrame drawFrame_;
	/** We store the LColor::color value as an int to get LColor.h out
	 *  of the header file.
	 */
	int frame_color_;
	///
	bool no_selection;
	///
	mutable lyx::paroffset_type old_par;

	///
	// to remember old painted frame dimensions to clear it on the right spot!
	///
	mutable bool in_insetAllowed;
	///
	// these are used to check for mouse movement in Motion selection code
	///
	int mouse_x;
	int mouse_y;
public:
	///
	mutable LyXText text_;
	///
	mutable int textwidth_;
};
#endif
