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

#include "CoordCache.h"
#include "Cursor.h"
#include "MetricsInfo.h"
#include "TextMetrics.h"
#include "update_flags.h"

#include "support/types.h"

#include <boost/tuple/tuple.hpp>
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
class Cursor;
class Text;
class ParIterator;
class ParagraphMetrics;
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
	/// returns the buffer currently set
	Buffer * setBuffer(Buffer * b);
	/// return the buffer being viewed.
	Buffer * buffer() const;

	/// resize the BufferView.
	void resize();

	/// perform pending metrics updates.
	/** \c Update::FitCursor means first to do a FitCursor, and to
	 * force an update if screen position changes.
	 * \c Update::Force means to force an update in any case.
	 * \retval true if a screen redraw is needed
	 */
	bool update(Update::flags flags = Update::FitCursor | Update::Force);

	/// move the screen to fit the cursor.
	/// Only to be called with good y coordinates (after a bv::metrics)
	bool fitCursor();
	/// reset the scrollbar to reflect current view position.
	void updateScrollbar();
	/// return the Scrollbar Parameters.
	ScrollbarParameters const & scrollbarParameters() const;

	/// Save the current position as bookmark.
	/// if idx == 0, save to temp_bookmark
	void saveBookmark(unsigned int idx);
	/// goto a specified position, try top_id first, and then bottom_pit
	/// return the bottom_pit and top_id of the new paragraph
	boost::tuple<pit_type, pos_type, int> moveToPosition(
		pit_type bottom_pit, ///< Paragraph pit, used when par_id is zero or invalid.
		pos_type bottom_pos, ///< Paragraph pit, used when par_id is zero or invalid.
		int top_id, ///< Paragraph ID, \sa Paragraph
		pos_type top_pos ///< Position in the \c Paragraph
		);
	/// return the current change at the cursor.
	Change const getCurrentChange() const;

	/// move cursor to the named label.
	void gotoLabel(docstring const & label);

	/// set the cursor based on the given TeX source row.
	void setCursorFromRow(int row);

	/// set cursor to the given inset. Return true if found.
	bool setCursorFromInset(Inset const *);

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

	/// translate and insert a character, using the correct keymap.
	void translateAndInsert(char_type c, Text * t, Cursor & cur);

	/// return true for events that will handle.
	FuncStatus getStatus(FuncRequest const & cmd);
	/// execute the given function.
	/// \return the Update::flags for further metrics update.
	Update::flags dispatch(FuncRequest const & argument);

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
	/// \retval true if a redraw is needed
	bool workAreaDispatch(FuncRequest const & ev);

	/// access to anchor.
	pit_type anchor_ref() const;

	/// access to full cursor.
	Cursor & cursor();
	/// access to full cursor.
	Cursor const & cursor() const;
	/// sets cursor.
	/// This will also open all relevant collapsable insets.
	void setCursor(DocIterator const &);
	/// Check deleteEmptyParagraphMechanism and update metrics if needed.
	/// \retval true if an update was needed.
	bool checkDepm(Cursor & cur, Cursor & old);
	/// sets cursor.
	/// This is used when handling LFUN_MOUSE_PRESS.
	bool mouseSetCursor(Cursor & cur, bool select = false);

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
	TextMetrics const & textMetrics(Text const * t) const;
	TextMetrics & textMetrics(Text const * t);
	///
	ParagraphMetrics const & parMetrics(Text const *, pit_type) const;

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
		Inset * inset)> showInsetDialog;

	/// This signal is emitted when some dialogs needs to be updated.
	boost::signal<void(std::string name,
		std::string data)> updateDialog;

	/// This signal is emitted when the layout at the cursor is changed.
	boost::signal<void(docstring layout)> layoutChanged;

private:
	///
	bool multiParSel();

	/// Search recursively for the the innermost inset that covers (x, y) position.
	/// \retval 0 if no inset is found.
	Inset const * getCoveringInset(
		Text const & text, //< The Text where we start searching.
		int x, //< x-coordinate on screen
		int y  //< y-coordinate on screen
		);

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
	Cursor cursor_;
	///
	bool multiparsel_cache_;
	///
	pit_type anchor_ref_;
	///
	int offset_ref_;
	///
	void updateOffsetRef();
	///
	bool need_centering_;

	/// keyboard mapping object.
	boost::scoped_ptr<Intl> const intl_;

	/// last visited inset (kept to send setMouseHover(false) )
	Inset * last_inset_;

	/// A map from a Text to the associated text metrics
	typedef std::map<Text const *, TextMetrics> TextMetricsCache;
	mutable TextMetricsCache text_metrics_;
};


} // namespace lyx

#endif // BUFFERVIEW_H
