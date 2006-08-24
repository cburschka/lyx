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

#include "metricsinfo.h"

#include "frontends/LyXKeySym.h"

#include "support/types.h"

#include <boost/utility.hpp>
#include <boost/signal.hpp>

#include <string>

class Buffer;
class Change;
class DocIterator;
class FuncRequest;
class FuncStatus;
class Language;
class LCursor;
class LyXText;
class LyXView;
class ParIterator;
class ViewMetricsInfo;

namespace Update {
	enum flags {
		FitCursor = 1,
		Force = 2,
		SinglePar = 4,
		MultiParSel = 8
	};

inline flags operator|(flags const f, flags const g)
{
	return static_cast<flags>(int(f) | int(g));
}

inline flags operator&(flags const f, flags const g)
{
	return static_cast<flags>(int(f) & int(g));
}

} // namespace

/// Scrollbar Parameters
struct ScrollbarParameters
{
	void reset(int h = 0, int p = 0, int l = 0)
	{
		height = h;
		position = p;
		lineScrollHeight = l;
	}

	/// The total document height in pixels
	int height;
	/// The current position in the document, in pixels
	int position;
	/// the line-scroll amount, in pixels
	int lineScrollHeight;
};

/**
 * A buffer view encapsulates a view onto a particular
 * buffer, and allows access to operate upon it. A view
 * is a sliding window of the entire document rendering.
 *
 * Eventually we will allow several views onto a single
 * buffer, but not yet.
 */
class BufferView : boost::noncopyable {
public:
	/**
	 * Create a view with the given owner main window,
	 * of the given dimensions.
	 */
	BufferView(LyXView * owner);

	~BufferView();

	/// set the buffer we are viewing
	void setBuffer(Buffer * b);
	/// return the buffer being viewed
	Buffer * buffer() const;
	/// return the first layout of the Buffer.
	std::string firstLayout();

	/// return the owning main view
	LyXView * owner() const;

	/// resize event has happened
	void resize();

	/// reload the contained buffer
	void reload();
	/// load a buffer into the view
	bool loadLyXFile(std::string const & name, bool tolastfiles = true);

	/** perform pending painting updates. \c fitcursor means first
	 *  to do a fitcursor, and to force an update if screen
	 *  position changes. \c forceupdate means to force an update
	 *  in any case.
	 * \return true if a full updateMetrics() is needed.
	 */
	bool update(Update::flags flags = Update::FitCursor | Update::Force);

	/// move the screen to fit the cursor. Only to be called with
	/// good y coordinates (after a bv::metrics)
	bool fitCursor();
	/// reset the scrollbar to reflect current view position
	void updateScrollbar();
	/// return the Scrollbar Parameters
	ScrollbarParameters const & scrollbarParameters() const;

	/// FIXME
	bool available() const;

	/// Save the current position as bookmark i
	void savePosition(unsigned int i);
	/// Restore the position from bookmark i
	void restorePosition(unsigned int i);
	/// does the given bookmark have a saved position ?
	bool isSavedPosition(unsigned int i);
	/// save bookmarks to .lyx/session
	void saveSavedPositions();

	/// return the current change at the cursor
	Change const getCurrentChange();

	/// return the lyxtext we are using
	LyXText * getLyXText();

	/// return the lyxtext we are using
	LyXText const * getLyXText() const;

	/// move cursor to the named label
	void gotoLabel(std::string const & label);

	/// set the cursor based on the given TeX source row
	void setCursorFromRow(int row);

	/// center the document view around the cursor
	void center();
	/// scroll document by the given number of lines of default height
	void scroll(int lines);
	/// Scroll the view by a number of pixels
	void scrollDocView(int pixels);
	/// Set the cursor position based on the scrollbar one.
	void setCursorFromScrollbar();

	/// return the pixel width of the document view
	int workWidth() const;
	/// return the pixel height of the document view
	int workHeight() const;

	/// switch between primary and secondary keymaps for RTL entry
	void switchKeyMap();

	/// return true for events that will handle
	FuncStatus getStatus(FuncRequest const & cmd);
	/// execute the given function
	bool dispatch(FuncRequest const & argument);

	///
	void selectionRequested();
	///
	void selectionLost();

	///
	void workAreaResize(int width, int height);

	/// a function should be executed from the workarea
	bool workAreaDispatch(FuncRequest const & ev);

	/// clear the X selection
	void unsetXSel();

	/// access to offset
	int offset_ref() const;
	/// access to anchor
	lyx::pit_type anchor_ref() const;

	/// access to full cursor
	LCursor & cursor();
	/// access to full cursor
	LCursor const & cursor() const;
	///
	LyXText * text() const;
	/// sets cursor and open all relevant collapsable insets.
	void setCursor(DocIterator const &);
	/// sets cursor; this is used when handling LFUN_MOUSE_PRESS.
	void mouseSetCursor(LCursor & cur);

	/* Sets the selection. When \c backwards == false, set anchor
	 * to \c cur and cursor to \c cur + \c length. When \c
	 * backwards == true, set anchor to \c cur and cursor to \c
	 * cur + \c length.
	 */
	void putSelectionAt(DocIterator const & cur,
		int length, bool backwards);
	///
	ViewMetricsInfo const & viewMetricsInfo();
	///
	void updateMetrics(bool singlepar = false);

	/// This signal is emitted when some message shows up.
	boost::signal<void(std::string)> message;

private:
	///
	class Pimpl;
	///
	friend class BufferView::Pimpl;
	///
	Pimpl * pimpl_;
};

#endif // BUFFERVIEW_H
