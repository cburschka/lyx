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
	void insetUnlock(BufferView *);
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *,
				UpdatableInset *, bool lr = false);
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const ;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const ;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::TEXT_CODE; }
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const;
	///
	int insetInInsetY() const;
	///
	void fitInsetCursor(BufferView *) const;
	///
	bool insertInset(BufferView *, InsetOld *);
	///
	bool insetAllowed(InsetOld::Code) const;
	///
	UpdatableInset * getLockingInset() const;
	///
	UpdatableInset * getFirstLockingInsetOfType(InsetOld::Code);
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
	void setFrameColor(LColor_color);
	///
	LyXText * getLyXText(BufferView const *,
			     bool const recursive = false) const;
	///
	void setViewCache(BufferView const * bv) const;
	///
	void deleteLyXText(BufferView *, bool recursive = true) const;
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
	InsetOld * getInsetFromID(int id) const;
	///
	ParagraphList * getParagraphs(int) const;
	///
	LyXText * getText(int) const;
	///
	LyXCursor const & cursor(BufferView *) const;
	///
	bool allowSpellcheck() const { return true; }
	///
	WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float & value) const;
	///
	void selectSelectedWord(BufferView *);

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
	/// find next change
	bool nextChange(BufferView *, lyx::pos_type & length);

	///
	bool searchForward(BufferView *, std::string const &,
			   bool = true, bool = false);
	///
	bool searchBackward(BufferView *, std::string const &,
			    bool = true, bool = false);
	///
	bool checkInsertChar(LyXFont &);
	///
	void getDrawFont(LyXFont &) const;
	/// append text onto the existing text
	void appendParagraphs(Buffer * bp, ParagraphList &);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	///
	bool haveParagraphs() const {
		return true;
	}
	///
	mutable ParagraphList paragraphs;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
	///
	void updateLocal(BufferView *, bool mark_dirty);
	/// set parameters for an initial lock of this inset
	void lockInset(BufferView *);
	/// lock an inset inside this one
	void lockInset(BufferView *, UpdatableInset *);

private:
	///
	void init();
	///
	void lfunMousePress(FuncRequest const &);
	///
	bool lfunMouseRelease(FuncRequest const &);
	///
	void lfunMouseMotion(FuncRequest const &);

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
	bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
				   mouse_button::state button = mouse_button::none);
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
	bool cboundary() const;
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
	mutable bool locked;
	///
	mutable int top_y;
	///
	lyx::paroffset_type inset_par;
	///
	lyx::pos_type inset_pos;
	///
	bool inset_boundary;
	///
	mutable int inset_x;
	///
	mutable int inset_y;
	///
	bool no_selection;
	///
	UpdatableInset * the_locking_inset;
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
