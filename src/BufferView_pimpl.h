// -*- C++ -*-
/**
 * \file BufferView_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Alfredo Braustein
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFERVIEW_PIMPL_H
#define BUFFERVIEW_PIMPL_H

#include "errorlist.h"
#include "BufferView.h"

#include "insets/inset.h"

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"
#include "frontends/Timeout.h"

#include "support/types.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


class Change;
class LyXView;
class WorkArea;
class LyXScreen;
class FuncRequest;

///
struct BufferView::Pimpl : public boost::signals::trackable {
	///
	Pimpl(BufferView * i, LyXView * o,
	      int xpos, int ypos, int width, int height);
	///
	Painter & painter() const;
	/// return the screen for this bview
	LyXScreen & screen() const;
	///
	void buffer(Buffer *);
	/// Return true if the cursor was fitted.
	bool fitCursor();
	///
	void redoCurrentBuffer();
	///
	void resizeCurrentBuffer();
	///
	void update();
	/**
	 * Repaint pixmap. Used for when we've made a visible
	 * change but don't need the full update() logic
	 */
	///
	bool newFile(string const &, string const &, bool);
	///
	bool loadLyXFile(string const &, bool);
	///
	void workAreaResize();
	///
	void updateScrollbar();
	///
	void scrollDocView(int value);
	/**
	 * Wheel mouse scroll, move by multiples of text->defaultRowHeight().
	 */
	void scroll(int lines);
	///
	void workAreaKeyPress(LyXKeySymPtr key, key_modifier::state state);
	///
	void selectionRequested();
	///
	void selectionLost();
	///
	void cursorToggle();
	///
	bool available() const;
	/// get the change at the cursor position
	Change const getCurrentChange();
	///
	void beforeChange(LyXText *);
	///
	void savePosition(unsigned int i);
	///
	void restorePosition(unsigned int i);
	///
	bool isSavedPosition(unsigned int i);
	///
	void switchKeyMap();
	///
	void insetUnlock();
	///
	void center();
	///
	bool insertInset(InsetOld * inset, string const & lout = string());
	///
	void updateInset(InsetOld const * inset);
	/// a function should be executed from the workarea
	bool workAreaDispatch(FuncRequest const & ev);
	/// a function should be executed
	bool dispatch(FuncRequest const & ev);
	///
	int top_y() const;
	///
	void top_y(int y);
private:
	/// the y coordinate of the top of the screen
	int top_y_;
	/// An error list (replaces the error insets)
	ErrorList errorlist_;
	/// add an error to the list
	void addError(ErrorItem const &);
	/// buffer errors signal connection
	boost::signals::connection errorConnection_;
	/// buffer messages signal connection
	boost::signals::connection messageConnection_;
	/// buffer busy status signal connection
	boost::signals::connection busyConnection_;
	/// buffer title changed signal connection
	boost::signals::connection titleConnection_;
	/// buffer reset timers signal connection
	boost::signals::connection timerConnection_;
	/// buffer readonly status changed signal connection
	boost::signals::connection readonlyConnection_;
	/// buffer closing signal connection
	boost::signals::connection closingConnection_;
	/// connect to signals in the given buffer
	void connectBuffer(Buffer & buf);
	/// disconnect from signals in the given buffer
	void disconnectBuffer();
	/// track changes for the document
	void trackChanges();
	/// notify readonly status
	void showReadonly(bool);

	/**
	 * Change all insets with the given code's contents to a new
	 * string. May only be used with InsetCommand-derived insets
	 * Returns true if a screen update is needed.
	 */
	bool ChangeInsets(InsetOld::Code code, string const & from,
			  string const & to);

	///
	friend class BufferView;

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
	/// Get next inset of this class from current cursor position
	InsetOld * getInsetByCode(InsetOld::Code code);
	///
	void MenuInsertLyXFile(string const & filen);
	/// our workarea
	WorkArea & workarea() const;
};
#endif // BUFFERVIEW_PIMPL_H
