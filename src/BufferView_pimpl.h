// -*- C++ -*-

#ifndef BUFFERVIEW_PIMPL_H
#define BUFFERVIEW_PIMPL_H

#include <boost/smart_ptr.hpp>

#include "BufferView.h"
#include "UpdateInset.h"
#include "commandtags.h"
#include "frontends/Timeout.h"
#include "WorkArea.h"
#include "paragraph.h"
#include "insets/insetspecialchar.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class WorkArea;
class LyXScreen;

///
struct BufferView::Pimpl : public SigC::Object {
	///
	Pimpl(BufferView * i, LyXView * o,
	      int xpos, int ypos, int width, int height);
	///
	Painter & painter();
	///
	void buffer(Buffer *);
	///
	void resize(int xpos, int ypos, int width, int height);
	///
	void resize();
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
	void workAreaExpose();
	///
	void updateScrollbar();
	///
	void scrollCB(double value);
	///
	Inset * checkInsetHit(LyXText *, int & x, int & y,
			      unsigned int button);
	/// 
	int scrollUp(long time);
	///
	int scrollDown(long time);
	///
	void workAreaKeyPress(KeySym, unsigned int state);
	///
	void workAreaMotionNotify(int x, int y, unsigned int state);
	///
	void workAreaButtonPress(int x, int y, unsigned int button);
	///
	void workAreaButtonRelease(int x, int y, unsigned int button);
	///
	void doubleClick(int x, int y, unsigned int button);
	///
	void tripleClick(int x, int y, unsigned int button);
	///
	void enterView();
	///
	void leaveView();
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
	void insetSleep();
	///
	void insetWakeup();
	///
	void insetUnlock();
	///
	bool focus() const;
	///
	void focus(bool);
	///
	bool active() const;
	///
	bool belowMouse() const;
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
	///
	friend class BufferView;
	/// open and lock an updatable inset
	bool open_new_inset(UpdatableInset * new_inset, bool behind = false);
	///
	void protectedBlank(LyXText * lt);
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
	long current_scrollbar_value;
	///
	Timeout cursor_timeout;
	///
	WorkArea workarea_;
	///
	UpdateInset updatelist;
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
                Paragraph::size_type par_pos;
		///
		Position() : par_id(0), par_pos(0) {}
		///
		Position(string const & f, int id, Paragraph::size_type pos)
                        : filename(f), par_id(id), par_pos(pos) {}
	};
	///
	std::vector<Position> saved_positions;
	///
	void moveCursorUpdate(bool selecting);
        /// Get next inset of this class from current cursor position  
	Inset * getInsetByCode(Inset::Code code);
	///
	void MenuInsertLyXFile(string const & filen);
	///
	bool inset_slept;
};
#endif
