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

#include "support/types.h"

#include <boost/utility.hpp>

#include <string>

class Buffer;
class Change;
class DocIterator;
class ErrorList;
class FuncRequest;
class FuncStatus;
class Language;
class LCursor;
class LyXText;
class LyXScreen;
class LyXView;
class Painter;
class ParIterator;

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
	BufferView(LyXView * owner, int w, int h);

	~BufferView();

	/// set the buffer we are viewing
	void setBuffer(Buffer * b);
	/// return the buffer being viewed
	Buffer * buffer() const;

	/// return the painter object for drawing onto the view
	Painter & painter() const;
	/// return the screen object for handling re-drawing
	LyXScreen & screen() const;
	/// return the owning main view
	LyXView * owner() const;

	/// resize event has happened
	void resize();

	/// reload the contained buffer
	void reload();
	/// create a new buffer based on template
	void newFile(std::string const & fname, std::string const & tname,
		     bool named = true);
	/// load a buffer into the view
	bool loadLyXFile(std::string const & name, bool tolastfiles = true);

	/** perform pending painting updates. \c fitcursor means first
	 *  to do a fitcursor, and to force an update if screen
	 *  position changes. \c forceupdate means to force an update
	 *  in any case.
	 */
	void update(bool fitcursor = true, bool forceupdate = true);
	/// move the screen to fit the cursor. Only to be called with
	/// good y coordinates (after a bv::metrics)
	bool fitCursor();
	/// reset the scrollbar to reflect current view position
	void updateScrollbar();

	/// FIXME
	bool available() const;

	/// Save the current position as bookmark i
	void savePosition(unsigned int i);
	/// Restore the position from bookmark i
	void restorePosition(unsigned int i);
	/// does the given bookmark have a saved position ?
	bool isSavedPosition(unsigned int i);

	/// return the current change at the cursor
	Change const getCurrentChange();

	/// return the lyxtext we are using
	LyXText * getLyXText();

	/// return the lyxtext we are using
	LyXText const * getLyXText() const;

	/// simple replacing. Use the font of the first selected character
	void replaceSelectionWithString(std::string const & str);

	/// move cursor to the named label
	void gotoLabel(std::string const & label);

	/// get the stored error list
	ErrorList const & getErrorList() const;
	/// show the error list to the user
	void showErrorList(std::string const &) const;
	/// set the cursor based on the given TeX source row
	void setCursorFromRow(int row);

	/// hide the cursor if it is visible
	void hideCursor();

	/// center the document view around the cursor
	void center();
	/// scroll document by the given number of lines of default height
	void scroll(int lines);
	/// Scroll the view by a number of pixels
	void scrollDocView(int pixels);

	/// return the pixel width of the document view
	int workWidth() const;
	/// return the pixel height of the document view
	int workHeight() const;

	/// switch between primary and secondary keymaps for RTL entry
	void switchKeyMap();

	/// get the contents of the window system clipboard
	std::string const getClipboard() const;
	/// fill the window system clipboard
	void stuffClipboard(std::string const &) const;
	/// tell the window system we have a selection
	void haveSelection(bool sel);

	/// return true for events that will handle
	FuncStatus getStatus(FuncRequest const & cmd);
	/// execute the given function
	bool dispatch(FuncRequest const & argument);

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
	///
	void setCursor(ParIterator const & par, lyx::pos_type pos);
	/* Sets the selection. When \c backwards == false, set anchor
	 * to \c cur and cursor to \c cur + \c length. When \c
	 * backwards == true, set anchor to \c cur and cursor to \c
	 * cur + \c length.
	 */
	void putSelectionAt(DocIterator const & cur,
		int length, bool backwards);


private:
	///
	class Pimpl;
	///
	friend class BufferView::Pimpl;
	///
	Pimpl * pimpl_;
};

#endif // BUFFERVIEW_H
