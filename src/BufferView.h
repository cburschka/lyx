// -*- C++ -*-
/**
 * \file BufferView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braustein
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_VIEW_H
#define BUFFER_VIEW_H

#include "coordcache.h"
#include "cursor.h"
#include "metricsinfo.h"
#include "UpdateFlags.h"
#include "support/types.h"

#include <boost/utility.hpp>
#include <boost/signal.hpp>

#include <utility>
#include <string>


namespace lyx {

namespace support { class FileName; }

class Buffer;
class Change;
class DocIterator;
class FuncRequest;
class FuncStatus;
class Intl;
class Language;
class LCursor;
class LyXText;
class ParIterator;
class ViewMetricsInfo;

/// Scrollbar Parameters.
struct ScrollbarParameters
{
	void reset(int h = 0, int p = 0, int l = 0)
	{
		height = h;
		position = p;
		lineScrollHeight = l;
	}

	/// Total document height in pixels.
	int height;
	/// Current position in the document in pixels.
	int position;
	/// Line-scroll amount in pixels.
	int lineScrollHeight;
};

/// Screen view of a Buffer.
/**
 * A BufferView encapsulates a view onto a particular
 * buffer, and allows access to operate upon it. A view
 * is a sliding window of the entire document rendering.
 * It is the official interface between the LyX core and
 * the frontend WorkArea.
 * 
 * \sa WorkArea
 * \sa Buffer
 * \sa CoordCache
 */
class BufferView : boost::noncopyable {
public:
	BufferView();

	~BufferView();

	/// set the buffer we are viewing.
	/// \todo FIXME: eventually, we will create a new BufferView
	/// when switching Buffers, so this method should go.
	void setBuffer(Buffer * b);
	/// return the buffer being viewed.
	Buffer * buffer() const;

	/// resize the BufferView.
	void resize();

	/// redisplay the referenced buffer.
	void reload();
	/// load a buffer into the view.
	bool loadLyXFile(support::FileName const & name, bool tolastfiles = true);

	/// perform pending painting updates.
	/** \c fitcursor means first
	 *  to do a fitcursor, and to force an update if screen
	 *  position changes. \c forceupdate means to force an update
	 *  in any case.
	 * \retval (false, xxx) if no redraw is required
	 * \retval (true, true) if a single paragraph redraw is needed
	 * \retval (true, false) if a full redraw is needed
	 */
	std::pair<bool, bool> update(Update::flags flags = Update::FitCursor | Update::Force);

	/// move the screen to fit the cursor.
	/// Only to be called with good y coordinates (after a bv::metrics)
	bool fitCursor();
	/// reset the scrollbar to reflect current view position.
	void updateScrollbar();
	/// return the Scrollbar Parameters.
	ScrollbarParameters const & scrollbarParameters() const;

	/// Save the current position as bookmark.
	/// if persistent=false, save to temp_bookmark
	void saveBookmark(bool persistent);
	/// goto a specified position.
	void moveToPosition(
		int par_id, ///< Paragraph ID, \sa Paragraph
		pos_type par_pos ///< Position in the \c Paragraph
		);
	/// return the current change at the cursor.
	Change const getCurrentChange() const;

	/// return the lyxtext we are using.
	LyXText * getLyXText();

	/// return the lyxtext we are using.
	LyXText const * getLyXText() const;

	/// move cursor to the named label.
	void gotoLabel(docstring const & label);

	/// set the cursor based on the given TeX source row.
	void setCursorFromRow(int row);

	/// center the document view around the cursor.
	void center();
	/// scroll document by the given number of lines of default height.
	void scroll(int lines);
	/// Scroll the view by a number of pixels.
	void scrollDocView(int pixels);
	/// Set the cursor position based on the scrollbar one.
	void setCursorFromScrollbar();

	/// return the pixel width of the document view.
	int workWidth() const;
	/// return the pixel height of the document view.
	int workHeight() const;

	/// switch between primary and secondary keymaps for RTL entry.
	void switchKeyMap();

	/// return true for events that will handle.
	FuncStatus getStatus(FuncRequest const & cmd);
	/// execute the given function.
	bool dispatch(FuncRequest const & argument);

	/// request an X11 selection.
	/// \return the selected string.
	docstring const requestSelection();
	/// clear the X11 selection.
	void clearSelection();

	/// resize method helper for \c WorkArea
	/// \sa WorkArea
	/// \sa resise
	void workAreaResize(int width, int height);

	/// dispatch method helper for \c WorkArea
	/// \sa WorkArea
	/// \retval (false, xxx) if no redraw is required
	/// \retval (true, true) if a single paragraph redraw is needed
	/// \retval (true, false) if a full redraw is needed
	std::pair<bool, bool> workAreaDispatch(FuncRequest const & ev);

	/// access to anchor.
	pit_type anchor_ref() const;

	/// access to full cursor.
	LCursor & cursor();
	/// access to full cursor.
	LCursor const & cursor() const;
	/// sets cursor.
	/// This will also open all relevant collapsable insets.
	void setCursor(DocIterator const &);
	/// sets cursor.
	/// This is used when handling LFUN_MOUSE_PRESS.
	void mouseSetCursor(LCursor & cur);

	/// sets the selection.
	/* When \c backwards == false, set anchor
	 * to \c cur and cursor to \c cur + \c length. When \c
	 * backwards == true, set anchor to \c cur and cursor to \c
	 * cur + \c length.
	 */
	void putSelectionAt(DocIterator const & cur,
		int length, bool backwards);

	/// return the internal \c ViewMetricsInfo.
	/// This is used specifically by the \c Workrea.
	/// \sa WorkArea
	/// \sa ViewMetricsInfo
	ViewMetricsInfo const & viewMetricsInfo();
	/// update the internal \c ViewMetricsInfo.
	/// \param singlepar indicates wether
	void updateMetrics(bool singlepar = false);

	///
	CoordCache & coordCache() {
		return coord_cache_;
	}
	///
	CoordCache const & coordCache() const {
		return coord_cache_;
	}
	/// get this view's keyboard map handler.
	Intl & getIntl() { return *intl_.get(); }
	///
	Intl const & getIntl() const { return *intl_.get(); }

	/// This signal is emitted when some message shows up.
	boost::signal<void(docstring)> message;

	/// This signal is emitted when some dialog needs to be shown.
	boost::signal<void(std::string name)> showDialog;

	/// This signal is emitted when some dialog needs to be shown with
	/// some data.
	boost::signal<void(std::string name,
		std::string data)> showDialogWithData;

	/// This signal is emitted when some inset dialogs needs to be shown.
	boost::signal<void(std::string name, std::string data,
		InsetBase * inset)> showInsetDialog;

	/// This signal is emitted when some dialogs needs to be updated.
	boost::signal<void(std::string name,
		std::string data)> updateDialog;

	/// This signal is emitted when the layout at the cursor is changed.
	boost::signal<void(std::string layout)> layoutChanged;

private:
	///
	bool multiParSel();
	///
	int width_;
	///
	int height_;
	///
	ScrollbarParameters scrollbarParameters_;

	///
	ViewMetricsInfo metrics_info_;
	///
	CoordCache coord_cache_;
	///
	Buffer * buffer_;

	/// Estimated average par height for scrollbar.
	int wh_;
	///
	void menuInsertLyXFile(std::string const & filen);

	/// this is used to handle XSelection events in the right manner.
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
	pit_type anchor_ref_;
	///
	int offset_ref_;

	/// keyboard mapping object.
	boost::scoped_ptr<Intl> const intl_;
};


} // namespace lyx

#endif // BUFFERVIEW_H
