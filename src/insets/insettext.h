// -*- C++ -*-
/**
 * \file insettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETTEXT_H
#define INSETTEXT_H

#include "updatableinset.h"
#include "textcursor.h"
#include "LString.h"
#include "LColor.h"
#include "ParagraphList.h"
#include "RowList.h"
#include "dimension.h"
#include "lyxtext.h"

#include "support/types.h"

#include "frontends/mouse_state.h"

#include <boost/shared_ptr.hpp>

class Painter;
class BufferView;
class Buffer;
class BufferParams;
class LyXCursor;
class LyXText;
class Row;
class Paragraph;

/**
 A text inset is like a TeX box to write full text
 (including styles and other insets) in a given space.
 @author: Jürgen Vigna
 */
class InsetText : public UpdatableInset {
public:
	///
	/// numbers need because of test if codeA < codeB
	///
	enum UpdateCodes {
		///
		NONE = 0,
		///
		CURSOR = 1,
		///
		DRAW_FRAME = 2,
		///
		SELECTION = 4,
		///
		CURSOR_PAR = 8,
		///
		FULL = 16,
		///
		INIT = 32
	};
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
	InsetText(BufferParams const &);
	///
	explicit InsetText(InsetText const &);
	///
	InsetBase * clone() const;
	///
	InsetText & operator=(InsetText const & it);
	/// empty inset to empty par, or just mark as erased
	void clear(bool just_mark_erased);
	///
	void read(Buffer const *, LyXLex &);
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	int textWidth(BufferView *, bool fordraw = false) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void update(BufferView *, bool = false);
	///
	void setUpdateStatus(BufferView *, int what) const;
	///
	string const editMessage() const;
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
	bool updateInsetInInset(BufferView *, Inset *);
	///
	RESULT localDispatch(FuncRequest const &);
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const ;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const { return Inset::TEXT_CODE; }
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const;
	///
	int insetInInsetY() const;
	///
	void fitInsetCursor(BufferView *) const;
	///
	bool insertInset(BufferView *, Inset *);
	///
	bool insetAllowed(Inset::Code) const;
	///
	UpdatableInset * getLockingInset() const;
	///
	UpdatableInset * getFirstLockingInsetOfType(Inset::Code);
	///
	void setFont(BufferView *, LyXFont const &,
		     bool toggleall = false,
		     bool selectall = false);
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	void init(InsetText const * ins);
	///
	void writeParagraphData(Buffer const *, std::ostream &) const;
	///
	void setParagraphData(ParagraphList const &);
	///
	void setText(string const &, LyXFont const &);
	///
	void setAutoBreakRows(bool);
	///
	bool getAutoBreakRows() const { return autoBreakRows; }
	///
	void setDrawFrame(BufferView *, DrawFrame);
	///
	void setFrameColor(BufferView *, LColor::color);
	///
	LyXText * getLyXText(BufferView const *,
			     bool const recursive = false) const;
	///
	void setViewCache(BufferView const * bv) const;
	///
	void deleteLyXText(BufferView *, bool recursive = true) const;
	///
	void resizeLyXText(BufferView *, bool force = false) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getLabelList(std::vector<string> &) const;
	///
	bool nodraw() const;
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
	void selectAll(BufferView * bv);
	///
	void clearSelection(BufferView * bv);
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
	bool searchForward(BufferView *, string const &,
			   bool = true, bool = false);
	///
	bool searchBackward(BufferView *, string const &,
			    bool = true, bool = false);
	///
	bool checkInsertChar(LyXFont &);
	///
	void getDrawFont(LyXFont &) const;
	/// append text onto the existing text
	void appendParagraphs(Buffer * bp, ParagraphList &);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

	//
	// Public structures and variables
	///
	mutable int need_update;

	bool haveParagraphs() const {
		return true;
	}
	///
	ParagraphList paragraphs;
protected:
	///
	void updateLocal(BufferView *, int what, bool mark_dirty);
	/// set parameters for an initial lock of this inset
	void lockInset(BufferView *);
	/// lock an inset inside this one
	void lockInset(BufferView *, UpdatableInset *);
	///
	mutable int drawTextXOffset;
	///
	mutable int drawTextYOffset;
	///
	bool autoBreakRows;
	///
	DrawFrame drawFrame_;
	///
	LColor::color frame_color;

private:
	///
	void lfunMousePress(FuncRequest const &);
	///
	bool lfunMouseRelease(FuncRequest const &);
	///
	void lfunMouseMotion(FuncRequest const &);

	///
	RESULT moveRight(BufferView *,
					 bool activate_inset = true,
					 bool selecting = false);
	///
	RESULT moveLeft(BufferView *,
					bool activate_inset = true,
					bool selecting = false);
	///
	RESULT moveRightIntern(BufferView *, bool front,
					       bool activate_inset = true,
					       bool selecting = false);
	///
	RESULT moveLeftIntern(BufferView *, bool front,
					      bool activate_inset = true,
					      bool selecting = false);

	///
	RESULT moveUp(BufferView *);
	///
	RESULT moveDown(BufferView *);
	///
	void setCharFont(Buffer const *, int pos, LyXFont const & font);
	///
	bool checkAndActivateInset(BufferView * bv, bool front);
	///
	bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
				   mouse_button::state button = mouse_button::none);
	///
	void removeNewlines();
	///
	int cx(BufferView *) const;
	///
	int cix(BufferView *) const;
	///
	int cy(BufferView *) const;
	///
	int ciy(BufferView *) const;
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
	void saveLyXTextState(LyXText *) const;
	///
	void restoreLyXTextState(LyXText *) const;
	///
	void collapseParagraphs(BufferView *);

	/* Private structures and variables */
	///
	mutable bool locked;
	///
	mutable Dimension dim_;
	///
	mutable int top_y;
	///
	ParagraphList::iterator inset_par;
	///
	lyx::pos_type inset_pos;
	///
	bool inset_boundary;
	///
	mutable int inset_x;
	///
	mutable int inset_y;
	///
	mutable int old_max_width;
	///
	bool no_selection;
	///
	UpdatableInset * the_locking_inset;
	///
	mutable ParagraphList::iterator old_par;
	///
	mutable int last_drawn_width;

	/// some funny 'temporarily saved state'
	mutable TextCursor sstate;

	///
	// to remember old painted frame dimensions to clear it on the right spot!
	///
	mutable int frame_x;
	mutable int frame_y;
	mutable int frame_w;
	mutable int frame_h;
	///
	bool in_update; /* as update is not reentrant! */
	mutable bool in_insetAllowed;
	///
	// these are used to check for mouse movement in Motion selection code
	///
	int mouse_x;
	int mouse_y;
public:
	///
	void reinitLyXText() const;
	///
	mutable LyXText text_;
};
#endif
