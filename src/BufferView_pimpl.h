// -*- C++ -*-
/**
 * \file BufferView_pimpl.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author various
 */

#ifndef BUFFERVIEW_PIMPL_H
#define BUFFERVIEW_PIMPL_H

#include "BufferView.h"
#include "commandtags.h"
#include "frontends/Timeout.h"
#include "frontends/key_state.h"
#include "frontends/mouse_state.h"
#include "frontends/LyXKeySym.h"
#include "box.h"
#include "insets/insetspecialchar.h"
#include "support/types.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class WorkArea;
class LyXScreen;
 
///
struct BufferView::Pimpl : public boost::signals::trackable {
	///
	Pimpl(BufferView * i, LyXView * o,
	      int xpos, int ypos, int width, int height);
	///
	Painter & painter() const;
	/// return the work area for this bview
	WorkArea & workarea() const;
	/// return the screen for this bview
	LyXScreen & screen() const;
	///
	void buffer(Buffer *);
	///
	void redraw();
	/// Return true if the cursor was fitted.
	bool fitCursor();
	///
	void redoCurrentBuffer();
	///
	int resizeCurrentBuffer();
	///
	void update();
	//
	void update(LyXText *, BufferView::UpdateCodes);
	/// Update pixmap of screen
	void updateScreen();
	///
	void workAreaResize();
	///
	void updateScrollbar();
	///
	void scrollDocView(int value);
	/**
	 * Returns an inset if inset was hit, or 0 if not.
	 *
	 * If hit, the coordinates are changed relative to the inset.
	 */
	Inset * checkInsetHit(LyXText *, int & x, int & y);
	/// wheel mouse scroll
	int scroll(long time);
	///
	void workAreaKeyPress(LyXKeySymPtr key,
			      key_modifier::state state);
	///
	void workAreaMotionNotify(int x, int y, mouse_button::state state);
	///
	void workAreaButtonPress(int x, int y, mouse_button::state button);
	///
	void workAreaButtonRelease(int x, int y, mouse_button::state button);
	///
	void doubleClick(int x, int y, mouse_button::state button);
	///
	void tripleClick(int x, int y, mouse_button::state button);
	///
	void selectionRequested();
	///
	void selectionLost();
	///
	void cursorToggle();
	///
	void cursorPrevious(LyXText *);
	///
	void cursorNext(LyXText *);
	///
	bool available() const;
	///
	void beforeChange(LyXText *);
	///
	void savePosition(unsigned int i);
	///
	void restorePosition(unsigned int i);
	///
	bool isSavedPosition(unsigned int i);
	///
	void setState();
	///
	void insetUnlock();
	///
	void showCursor();
	///
	void hideCursor();
	///
	void toggleSelection(bool = true);
	///
	void toggleToggle();
	///
	void center();
	///
	bool insertInset(Inset * inset, string const & lout = string());
	///
	void updateInset(Inset * inset, bool mark_dirty);
	///
	bool Dispatch(kb_action action, string const & argument);
private:
	/**
	 * Return the on-screen dimensions of the inset at the cursor.
	 * Pre-condition: the cursor must be at an inset.
	 */
	Box insetDimensions(LyXText const & text, LyXCursor const & cursor) const;
	/**
	 * check if the given co-ordinates are inside an inset at the given cursor,
	 * if one exists. If so, the inset is returned, and the co-ordinates are
	 * made relative. Otherwise, 0 is returned.
	 */
	Inset * checkInset(LyXText const & text, LyXCursor const & cursor, int & x, int & y) const;
	///
	friend class BufferView;
	/// open and lock an updatable inset
	bool open_new_inset(UpdatableInset * new_inset, bool behind = false);
	///
	void specialChar(InsetSpecialChar::Kind);
	///
	void newline();
	///
	void hfill();
	///
	void smartQuote();
	///
	void insertAndEditInset(Inset *);
	///
	void gotoInset(std::vector<Inset::Code> const & codes,
		       bool same_content);
	///
	void gotoInset(Inset::Code codes, bool same_content);
	///
	BufferView * bv_;
	///
	LyXView * owner_;
	///
	Buffer * buffer_;
	///
	boost::scoped_ptr<LyXScreen> screen_;
	///
	boost::scoped_ptr<WorkArea> workarea_;
	///
	Timeout cursor_timeout;
	///
	void pasteClipboard(bool asPara);
	///
	void stuffClipboard(string const &) const;
	///
	bool using_xterm_cursor;
	///
	struct Position {
		/// Filename
		string filename;
		/// Cursor paragraph Id
		int par_id;
		/// Cursor position
		lyx::pos_type par_pos;
		///
		Position() : par_id(0), par_pos(0) {}
		///
		Position(string const & f, int id, lyx::pos_type pos)
			: filename(f), par_id(id), par_pos(pos) {}
	};
	///
	std::vector<Position> saved_positions;
	///
	void moveCursorUpdate(bool selecting, bool fitcur = true);
	/// Get next inset of this class from current cursor position
	Inset * getInsetByCode(Inset::Code code);
	///
	void MenuInsertLyXFile(string const & filen);
};
#endif // BUFFERVIEW_PIMPL_H
