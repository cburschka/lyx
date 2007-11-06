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

#include "update_flags.h"

#include "support/strfwd.h"
#include "support/types.h"

namespace lyx {

namespace support { class FileName; }

namespace frontend { class Painter; }
namespace frontend { class GuiBufferViewDelegate; }

class Buffer;
class Change;
class CoordCache;
class Cursor;
class DocIterator;
class FuncRequest;
class FuncStatus;
class Intl;
class Inset;
class ParIterator;
class ParagraphMetrics;
class Point;
class Text;
class TextMetrics;
class ViewMetricsInfo;

enum CursorStatus {
	CUR_INSIDE,
	CUR_ABOVE,
	CUR_BELOW
};

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
class BufferView {
public:
	///
	BufferView(Buffer & buffer);
	///
	~BufferView();

	/// return the buffer being viewed.
	Buffer & buffer();
	Buffer const & buffer() const;

	/// perform pending metrics updates.
	/** \c Update::FitCursor means first to do a FitCursor, and to
	 * force an update if screen position changes.
	 * \c Update::Force means to force an update in any case.
	 * \retval true if a screen redraw is needed
	 */
	void processUpdateFlags(Update::flags flags);

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
	/// goto a specified position, try top_id first, and then bottom_pit.
	/// \return true if success
	bool moveToPosition(
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

	/// center the document view around the cursor.
	void center();
	/// scroll down document by the given number of pixels.
	void scrollDown(int pixels);
	/// scroll up document by the given number of pixels.
	void scrollUp(int pixels);
	/// scroll document by the given number of pixels.
	void scroll(int pixels);
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

	/// resize the BufferView.
	/// \sa WorkArea
	void resize(int width, int height);

	/// dispatch method helper for \c WorkArea
	/// \sa WorkArea
	void mouseEventDispatch(FuncRequest const & ev);

	/// access to anchor.
	pit_type anchor_ref() const;

	///
	CursorStatus cursorStatus(DocIterator const & dit) const;
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
	void updateMetrics();

	///
	TextMetrics const & textMetrics(Text const * t) const;
	TextMetrics & textMetrics(Text const * t);
	///
	ParagraphMetrics const & parMetrics(Text const *, pit_type) const;

	///
	CoordCache & coordCache();
	///
	CoordCache const & coordCache() const;

	///
	Point getPos(DocIterator const & dit, bool boundary) const;


	///
	void draw(frontend::Painter & pain);

	/// get this view's keyboard map handler.
	Intl & getIntl();
	///
	Intl const & getIntl() const;

	//
	// Messages to the GUI
	//
	/// This signal is emitted when some message shows up.
	void message(docstring const & msg);

	/// This signal is emitted when some dialog needs to be shown.
	void showDialog(std::string const & name);

	/// This signal is emitted when some dialog needs to be shown with
	/// some data.
	void showDialogWithData(std::string const & name, std::string const & data);

	/// This signal is emitted when some inset dialogs needs to be shown.
	void showInsetDialog(std::string const & name, std::string const & data,
		Inset * inset);

	/// This signal is emitted when some dialogs needs to be updated.
	void updateDialog(std::string const & name, std::string const & data);

	///
	void setGuiDelegate(frontend::GuiBufferViewDelegate *);

	///
	docstring contentsOfPlaintextFile(std::string const & f, bool asParagraph);
	// Insert plain text file (if filename is empty, prompt for one)
	void insertPlaintextFile(std::string const & fileName, bool asParagraph);

private:
	/// noncopyable
	BufferView(BufferView const &);
	void operator=(BufferView const &);

	// the position relative to (0, baseline) of outermost paragraph
	Point coordOffset(DocIterator const & dit, bool boundary) const;
	/// Update current paragraph metrics.
	/// \return true if no further update is needed.
	bool singleParUpdate();
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
	Buffer & buffer_;

	///
	void menuInsertLyXFile(std::string const & filen);

	void updateOffsetRef();

	struct Private;
	Private * const d;
};

/// some space for drawing the 'nested' markers (in pixel)
inline int nestMargin() { return 15; }

/// margin for changebar
inline int changebarMargin() { return 12; }

/// right margin
inline int rightMargin() { return 10; }

} // namespace lyx

#endif // BUFFERVIEW_H
