// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 *
 *======================================================
 */
// The pristine updatable inset: Text


#ifndef INSETTEXT_H
#define INSETTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include "LString.h"
#include "LColor.h"
#include "paragraph.h"
#include "lyxcursor.h"

#include <boost/smart_ptr.hpp>

class Painter;
class BufferView;
class Buffer;
class BufferParams;
class LyXCursor;
class LyXText;
class LyXScreen;
class Row;

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
		CLEAR_FRAME = 2,
		///
		DRAW_FRAME = 4,
		///
		SELECTION = 8,
		///
		CURSOR_PAR = 16,
		///
		FULL = 32,
		///
		INIT = 64
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
	explicit
	InsetText(InsetText const &, bool same_id = false);
	///
	~InsetText();
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	InsetText & operator=(InsetText const & it);
	///
	void clear();
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
	int textWidth(BufferView *, bool fordraw = false) const;
	///
	void draw(BufferView *, LyXFont const &, int , float &, bool) const;
	///
	void update(BufferView *, LyXFont const &, bool = false);
	///
	void setUpdateStatus(BufferView *, int what) const;
	///
	string const editMessage() const;
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView *, bool front = true);
	///
	bool isTextInset() const { return true; }
	///
	bool doClearArea() const;
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
	bool insetButtonRelease(BufferView *, int, int, int);
	///
	void insetButtonPress(BufferView *, int, int, int);
	///
	void insetMotionNotify(BufferView *, int, int, int);
	///
	void insetKeyPress(XKeyEvent *);
	///
	UpdatableInset::RESULT localDispatch(BufferView *,
					     kb_action, string const &);
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docbook(Buffer const *, std::ostream &) const ;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const { return Inset::TEXT_CODE; }
	///
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	int insetInInsetY() const;
	///
	void toggleInsetCursor(BufferView *);
	///
	void showInsetCursor(BufferView *, bool show = true);
	///
	void hideInsetCursor(BufferView *);
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
	void init(InsetText const * ins = 0, bool same_id = false);
	///
	void writeParagraphData(Buffer const *, std::ostream &) const;
	///
	void setParagraphData(Paragraph *, bool same_id = false);
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
	void deleteLyXText(BufferView *, bool recursive = true) const;
	///
	void resizeLyXText(BufferView *, bool force = false) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	std::vector<string> const getLabelList() const;
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
	Paragraph * paragraph() const;
	///
	void paragraph(Paragraph *);
	///
	bool allowSpellcheck() { return true; }
	///
	string const selectNextWordToSpellcheck(BufferView *,
						float & value) const;
	void selectSelectedWord(BufferView *);
	///
	void toggleSelection(BufferView *, bool kill_selection);
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
	///
	void appendParagraphs(BufferParams const & bparams, Paragraph *);
	///
	//
	// Public structures and variables
	///
	mutable int need_update;

protected:
	///
	void updateLocal(BufferView *, int what, bool mark_dirty) const;
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
	struct InnerCache {
		///
		InnerCache(boost::shared_ptr<LyXText>);
		///
		boost::shared_ptr<LyXText> text;
		///
		bool remove;
	};
	///
	typedef std::map<BufferView *, InnerCache> Cache;
	///
	typedef Cache::value_type value_type;
	///
	int beginningOfMainBody(Buffer const *, Paragraph * par) const;
	///
	UpdatableInset::RESULT moveRight(BufferView *,
					 bool activate_inset = true,
					 bool selecting = false);
	///
	UpdatableInset::RESULT moveLeft(BufferView *,
					bool activate_inset = true,
					bool selecting = false);
	///
	UpdatableInset::RESULT moveRightIntern(BufferView *, bool front,
					       bool activate_inset = true,
					       bool selecting = false);
	///
	UpdatableInset::RESULT moveLeftIntern(BufferView *, bool front,
					      bool activate_inset = true,
					      bool selecting = false);

	///
	UpdatableInset::RESULT moveUp(BufferView *);
	///
	UpdatableInset::RESULT moveDown(BufferView *);
	///
	void setCharFont(Buffer const *, int pos, LyXFont const & font);
	///
	bool checkAndActivateInset(BufferView * bv, bool front);
	///
	bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
				   int button = 0);
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
	lyx::pos_type cpos(BufferView *) const;
	///
	Paragraph * cpar(BufferView *) const;
	///
	bool cboundary(BufferView *) const;
	///
	Row * crow(BufferView *) const;
	///
	void drawFrame(Painter &, bool cleared) const;
	///
	void clearFrame(Painter &, bool cleared) const;
	///
	void clearInset(BufferView *, int baseline, bool & cleared) const;
	///
	void saveLyXTextState(LyXText *) const;
	///
	void restoreLyXTextState(BufferView *, LyXText *) const;
	///
	void reinitLyXText() const;
	///
	void collapseParagraphs(BufferView *) const;

	/* Private structures and variables */
	///
	Paragraph * par;
	///
	mutable bool locked;
	///
	mutable int insetAscent;
	///
	mutable int insetDescent;
	///
	mutable int insetWidth;
	///
	mutable int top_y;
	///
	Paragraph * inset_par;
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
	mutable float xpos;
	///
	UpdatableInset * the_locking_inset;
	///
	mutable Paragraph * old_par;
	/// The cache.
	mutable Cache cache;
	///
	mutable int last_drawn_width;
	///
	mutable bool frame_is_visible;
	///
	mutable BufferView * cached_bview;
	///
	mutable boost::shared_ptr<LyXText> cached_text;
	///
	struct save_state {
		Paragraph * lpar;
		Paragraph * selstartpar;
		Paragraph * selendpar;
		lyx::pos_type pos;
		lyx::pos_type selstartpos;
		lyx::pos_type selendpos;
		bool boundary;
		bool selstartboundary;
		bool selendboundary;
		bool selection;
		bool mark_set;
		bool refresh;
	};
	///
	mutable save_state sstate;

	///
	// this is needed globally so we know that we're using it actually and
	// so the LyXText-Cache is not erased until used!
	mutable LyXText * lt;
	///
	// to remember old painted frame dimensions to clear it on the right spot!
	///
	mutable int frame_x;
	mutable int frame_y;
	mutable int frame_w;
	mutable int frame_h;
	///
	bool in_update; /* as update is not reentrant! */
	mutable BufferView * do_resize;
	mutable bool do_reinit;
	mutable bool in_insetAllowed;
	///
	// these are used to check for mouse movement in Motion selection code
	///
	int mouse_x;
	int mouse_y;
};
#endif
