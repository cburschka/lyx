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

#include "lyxinset.h"
#include "LString.h"
#include "lyxcursor.h"

class Painter;
class BufferView;
class Buffer;
class LyXCursor;
class LyXParagraph;
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
	Inset * Clone(Buffer const &) const;
	///
	InsetText & operator=(InsetText const & it);
	///
	void clear();
	///
	void Read(Buffer const *, LyXLex &);
	///
	void Write(Buffer const *, std::ostream &) const;
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
	void SetUpdateStatus(BufferView *, int what) const;
	///
	string const EditMessage() const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	bool IsTextInset() const { return true; }
	///
	bool doClearArea() const;
	///
	void InsetUnlock(BufferView *);
	///
	bool LockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool UnlockInsetInInset(BufferView *, UpdatableInset *, bool lr = false);
	///
	bool UpdateInsetInInset(BufferView *, Inset *);
	///
	void InsetButtonRelease(BufferView *, int, int, int);
	///
	void InsetButtonPress(BufferView *, int, int, int);
	///
	void InsetMotionNotify(BufferView *, int, int, int);
	///
	void InsetKeyPress(XKeyEvent *);
	///
	UpdatableInset::RESULT LocalDispatch(BufferView *, kb_action, string const &);
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int DocBook(Buffer const *, std::ostream &) const ;
	///
	void Validate(LaTeXFeatures & features) const;
	///
	Inset::Code LyxCode() const { return Inset::TEXT_CODE; }
	///
	void GetCursorPos(BufferView *, int & x, int & y) const;
	///
	unsigned int InsetInInsetY();
	///
	void ToggleInsetCursor(BufferView *);
	///
	bool InsertInset(BufferView *, Inset *);
	///
	UpdatableInset * GetLockingInset();
	///
	UpdatableInset * GetFirstLockingInsetOfType(Inset::Code);
	///
	void SetFont(BufferView *, LyXFont const &, bool toggleall = false,
	             bool selectall = false);
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	void init(InsetText const * ins = 0);
	///
	void WriteParagraphData(Buffer const *, std::ostream &) const;
	///
	void SetParagraphData(LyXParagraph *);
	///
	void SetText(string const &);
	///
	void SetAutoBreakRows(bool);
	///
	void SetDrawFrame(BufferView *, DrawFrame);
	///
	void SetFrameColor(BufferView *, LColor::color);
	///
	LyXText * getLyXText(BufferView const *, bool const recursive=false) const;
	///
	void deleteLyXText(BufferView *, bool recursive=true) const;
	///
	void resizeLyXText(BufferView *, bool force=false) const;
	///
	bool ShowInsetDialog(BufferView *) const;
	///
	std::vector<string> const getLabelList() const;
	///
	bool nodraw() const;
	///
	int scroll(bool recursive=true) const;
	void scroll(BufferView *bv, float sx) const {
		UpdatableInset::scroll(bv, sx);
	}
	void scroll(BufferView *bv, int offset) const {
		UpdatableInset::scroll(bv, offset);
	}
	///
	void selectAll(BufferView *bv);
	///
	void clearSelection(BufferView *bv);

	LyXParagraph * par;
	///
	mutable int need_update;

protected:
	///
	void UpdateLocal(BufferView *, int what, bool mark_dirty);
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
	int BeginningOfMainBody(Buffer const *, LyXParagraph * par) const;
	///
	void ShowInsetCursor(BufferView *, bool show=true);
	///
	void HideInsetCursor(BufferView *);
	///
	UpdatableInset::RESULT moveRight(BufferView *, bool activate_inset = true,
					 bool selecting = false);
	///
	UpdatableInset::RESULT moveLeft(BufferView *, bool activate_inset = true,
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
	void SetCharFont(Buffer const *, int pos, LyXFont const & font);
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
	LyXParagraph::size_type cpos(BufferView *) const;
	///
	LyXParagraph * cpar(BufferView *) const;
	///
	bool cboundary(BufferView *) const;
	///
	Row * crow(BufferView *) const;
	///
	/// This instead of a macro
	LyXText * TEXT(BufferView * bv) const {
		return getLyXText(bv);
	}
	///
	void drawFrame(Painter &, bool cleared) const;
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
	LyXParagraph * inset_par;
	///
	LyXParagraph::size_type inset_pos;
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
	LyXParagraph * old_par;
	/// The cache.
	mutable Cache cache;
	///
	mutable int last_drawn_width;
	///
	mutable bool frame_is_visible;
};
#endif
