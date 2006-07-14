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

#include "BufferView.h"
#include "cursor.h"
#include "metricsinfo.h"

#include "frontends/LyXKeySym.h"

#include "support/types.h"

class Change;
class LyXView;

class FuncRequest;
class FuncStatus;

namespace lyx {
namespace frontend {
class Gui;
}
}


///
class BufferView::Pimpl {
public:
	///
	Pimpl(BufferView & bv, LyXView * owner);
	///
	void setBuffer(Buffer * buf);
	/// return the first layout of the Buffer.
	std::string firstLayout();
	///
	void resizeCurrentBuffer();
	//
	bool fitCursor();
	//
	bool multiParSel();
	///
	bool update(Update::flags flags = Update::Force);
	/// load a buffer into the view
	bool loadLyXFile(std::string const &, bool);
	///
	void workAreaResize(int width, int height);
	///
	void updateScrollbar();
	///
	ScrollbarParameters const & scrollbarParameters() const;
	///
	void scrollDocView(int value);
	/// Wheel mouse scroll, move by multiples of text->defaultRowHeight().
	void scroll(int lines);
	///
	void workAreaKeyPress(LyXKeySymPtr key, key_modifier::state state);
	///
	void selectionRequested();
	///
	void selectionLost();
	///
	bool available() const;
	/// get the change at the cursor position
	Change const getCurrentChange();
	///
	void savePosition(unsigned int i);
	///
	void restorePosition(unsigned int i);
	///
	bool isSavedPosition(unsigned int i);
	/// save bookmarks to .lyx/session
	void saveSavedPositions();
	///
	void switchKeyMap();
	///
	void center();
	/// a function should be executed from the workarea
	bool workAreaDispatch(FuncRequest const & ev);
	/// return true for events that will handle
	FuncStatus getStatus(FuncRequest const & cmd);
	/// a function should be executed
	bool dispatch(FuncRequest const & ev);

	/// the frontend
	lyx::frontend::Gui & gui() const;

	/// Width and height of the BufferView in Pixels
	/**
	This is set externally by the workAreaResize method.
	*/
	int width() const;
	/// Height of the BufferView in Pixels
	/**
	This is set externally by the workAreaResize method.
	*/
	int height() const;

	///
	ViewMetricsInfo const & viewMetricsInfo();
	///
	void updateMetrics(bool singlepar = false);

private:
	///
	int width_;
	///
	int height_;
	///
	ScrollbarParameters scrollbarParameters_;

	/// track changes for the document
	void trackChanges();

	///
	ViewMetricsInfo metrics_info_;

	///
	friend class BufferView;

	///
	BufferView * bv_;
	///
	LyXView * owner_;
	///
	Buffer * buffer_;

	/// Estimated average par height for scrollbar
	int wh_;
	///
	class Position {
	public:
		/// Filename
		std::string filename;
		/// Cursor paragraph Id
		int par_id;
		/// Cursor position
		lyx::pos_type par_pos;
		///
		Position() : par_id(0), par_pos(0) {}
		///
		Position(std::string const & f, int id, lyx::pos_type pos)
			: filename(f), par_id(id), par_pos(pos) {}
	};
	///
	std::vector<Position> saved_positions;
	///
	void menuInsertLyXFile(std::string const & filen);

	/// this is used to handle XSelection events in the right manner
	struct {
		CursorSlice cursor;
		CursorSlice anchor;
		bool set;
	} xsel_cache_;
	///
	LCursor cursor_;
	///
	bool multiparsel_cache_;
	///
	lyx::pit_type anchor_ref_;
	///
	int offset_ref_;

};
#endif // BUFFERVIEW_PIMPL_H
