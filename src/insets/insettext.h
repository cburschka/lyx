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
#include "lyxcursor.h"

class Painter;
class BufferView;
class Buffer;
class LyXCursor;
class Paragraph;
class LColor;
class LyXText;
class LyXScreen;

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
	InsetText();
	///
	explicit
	InsetText(InsetText const &);
	///
	~InsetText();
	///
	Inset * clone(Buffer const &) const;
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
	int textWidth(BufferView *) const;
	///
	void draw(BufferView *, LyXFont const &, int , float &, bool) const;
	///
	void update(BufferView *, LyXFont const &, bool =false);
	///
	void setUpdateStatus(BufferView *, int what) const;
	///
	string const editMessage() const;
	///
	void edit(BufferView *, int, int, unsigned int);
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
	void insetButtonRelease(BufferView *, int, int, int);
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
	int docBook(Buffer const *, std::ostream &) const ;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const { return Inset::TEXT_CODE; }
	///
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	unsigned int insetInInsetY();
	///
	void toggleInsetCursor(BufferView *);
	///
	bool insertInset(BufferView *, Inset *);
	///
	UpdatableInset * getLockingInset();
	///
	UpdatableInset * getFirstLockingInsetOfType(Inset::Code);
	///
	void setFont(BufferView *, LyXFont const &,
		     bool toggleall = false,
	             bool selectall = false);
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	void init(InsetText const * ins = 0);
	///
	void writeParagraphData(Buffer const *, std::ostream &) const;
	///
	void setParagraphData(Paragraph *);
	///
	void setText(string const &);
	///
	void setAutoBreakRows(bool);
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
	void selectAll(BufferView *bv);
	///
	void clearSelection(BufferView *bv);

	Paragraph * par;
	///
	mutable int need_update;

protected:
	///
	void updateLocal(BufferView *, int what, bool mark_dirty);
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
	typedef std::map<BufferView *, LyXText *> Cache;
	///
	typedef Cache::value_type value_type;
	///
	int beginningOfMainBody(Buffer const *, Paragraph * par) const;
	///
	void showInsetCursor(BufferView *, bool show=true);
	///
	void hideInsetCursor(BufferView *);
	///
	UpdatableInset::RESULT moveRight(BufferView *,
					 bool activate_inset = true,
					 bool selecting = false);
	///
	UpdatableInset::RESULT moveLeft(BufferView *,
					bool activate_inset = true,
					bool selecting = false);
	///
	UpdatableInset::RESULT moveRightIntern(BufferView *, bool behind,
					       bool activate_inset = true,
					       bool selecting = false);
	///
	UpdatableInset::RESULT moveLeftIntern(BufferView *, bool behind, 
					      bool activate_inset = true,
					      bool selecting = false);

	///
	UpdatableInset::RESULT moveUp(BufferView *);
	///
	UpdatableInset::RESULT moveDown(BufferView *);
	///
	void setCharFont(Buffer const *, int pos, LyXFont const & font);
	///
	string const getText(int);
	///
	bool checkAndActivateInset(BufferView * bv, bool behind);
	///
	bool checkAndActivateInset(BufferView * bv, int x = 0, int y = 0,
				   int button = 0);
	///
	void removeNewlines();
	///
	int cx(BufferView *) const;
	///
	int cy(BufferView *) const;
	///
	Paragraph::size_type cpos(BufferView *) const;
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
	void clearInset(Painter &, int baseline, bool & cleared) const;
	
	/* Private structures and variables */
	///
	mutable bool locked;
	///
	mutable int insetAscent;
	///
	mutable int insetDescent;
	///
	mutable int insetWidth;
	///
	mutable int last_width;
	///
	mutable int last_height;
	///
	mutable int top_y;
	///
	Paragraph * inset_par;
	///
	Paragraph::size_type inset_pos;
	///
	bool inset_boundary;
	///
	mutable int inset_x;
	///
	mutable int inset_y;
	///
	mutable unsigned int old_max_width;
	///
	bool no_selection;
	///
	mutable float xpos;
	///
	UpdatableInset * the_locking_inset;
	///
	Paragraph * old_par;
	/// The cache.
	mutable Cache cache;
	///
	mutable int last_drawn_width;
	///
	mutable bool frame_is_visible;
};
#endif
