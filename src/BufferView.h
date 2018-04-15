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

#include "DocumentClassPtr.h"
#include "TexRow.h"
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
class CursorSlice;
class DispatchResult;
class DocIterator;
class DocumentClass;
class FuncRequest;
class FuncStatus;
class Intl;
class Inset;
class Length;
class ParIterator;
class ParagraphMetrics;
class Point;
class Row;
class TexRow;
class Text;
class TextMetrics;

enum CursorStatus {
	CUR_INSIDE,
	CUR_ABOVE,
	CUR_BELOW
};

/// Scrollbar Parameters.
struct ScrollbarParameters
{
	// These parameters are normalized against the screen geometry and pixel
	// coordinates. Position 0 corresponds to the top the the screen.
	ScrollbarParameters()
		: min(0), max(0), single_step(1), page_step(1)
	{}
	/// Minimum scrollbar position in pixels.
	int min;
	/// Maximum scrollbar position in pixels.
	int max;
	/// Line-scroll amount in pixels.
	int single_step;
	/// Page-scroll amount in pixels.
	int page_step;
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
	explicit BufferView(Buffer & buffer);
	///
	~BufferView();

	/// return the buffer being viewed.
	Buffer & buffer();
	Buffer const & buffer() const;

	///
	void setFullScreen(bool full_screen) { full_screen_ = full_screen; }

	/// right margin
	int rightMargin() const;

	/// left margin
	int leftMargin() const;

	/// return the on-screen size of this length
	/*
	 *  This is a wrapper around Length::inPixels that uses the
	 *  bufferview width as width and the EM value of the default
	 *  document font.
	 */
	int inPixels(Length const & len) const;

	/// \return true if the BufferView is at the top of the document.
	bool isTopScreen() const;

	/// \return true if the BufferView is at the bottom of the document.
	bool isBottomScreen() const;

	/// Add \p flags to current update flags and trigger an update.
	/* If this method is invoked several times before the update
	 * actually takes place, the effect is cumulative.
	 * \c Update::FitCursor means first to do a FitCursor, and to
	 * force an update if screen position changes.
	 * \c Update::Force means to force an update in any case.
	 */
	void processUpdateFlags(Update::flags flags);

	/// return true if one shall move the screen to fit the cursor.
	/// Only to be called with good y coordinates (after a bv::metrics)
	bool needsFitCursor() const;

	/// returns true if this row needs to be repainted (to erase caret)
	bool needRepaint(Text const * text, Row const & row) const;

	// Returns the amount of horizontal scrolling applied to the
	// top-level row where the cursor lies
	int horizScrollOffset() const;
	// Returns the amount of horizontal scrolling applied to the
	// row of text starting at (pit, pos)
	int horizScrollOffset(Text const * text,
	                      pit_type pit, pos_type pos) const;

	// Returns true if the row of text starting at (pit, pos) was scrolled
	// at the last draw event.
	bool hadHorizScrollOffset(Text const * text,
                              pit_type pit, pos_type pos) const;

	/// reset the scrollbar to reflect current view position.
	void updateScrollbar();
	/// return the Scrollbar Parameters.
	ScrollbarParameters const & scrollbarParameters() const;
	/// \return Tool tip for the given position.
	docstring toolTip(int x, int y) const;
	/// \return the context menu for the given position.
	std::string contextMenu(int x, int y) const;

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
	bool setCursorFromRow(int row);
	/// set the cursor based on the given start and end TextEntries.
	bool setCursorFromEntries(TexRow::TextEntry start, TexRow::TextEntry end);

	/// set cursor to the given inset. Return true if found.
	bool setCursorFromInset(Inset const *);
	/// Recenters the BufferView such that the passed cursor
	/// is in the center.
	void recenter();
	/// Ensure that the BufferView cursor is visible.
	/// This method will automatically scroll and update the BufferView
	/// (metrics+drawing) if needed.
	void showCursor();
	/// Ensure the passed cursor \p dit is visible.
	/// This method will automatically scroll and update the BufferView
	/// (metrics+drawing) if needed.
	/// \param recenter Whether the cursor should be centered on screen
	void showCursor(DocIterator const & dit, bool recenter,
		bool update);
	/// Scroll to the cursor.
	void scrollToCursor();
	/// Scroll to the cursor.
	/// \param recenter Whether the cursor should be centered on screen
	bool scrollToCursor(DocIterator const & dit, bool recenter);
	/// scroll down document by the given number of pixels.
	int scrollDown(int pixels);
	/// scroll up document by the given number of pixels.
	int scrollUp(int pixels);
	/// scroll document by the given number of pixels.
	int scroll(int pixels);
	/// Scroll the view by a number of pixels.
	void scrollDocView(int pixels, bool update);
	/// Set the cursor position based on the scrollbar one.
	void setCursorFromScrollbar();

	/// return the pixel width of the document view.
	int workWidth() const;
	/// return the pixel height of the document view.
	int workHeight() const;

	/// return the inline completion postfix.
	docstring const & inlineCompletion() const;
	/// return the number of unique characters in the inline completion.
	size_t const & inlineCompletionUniqueChars() const;
	/// return the position in the buffer of the inline completion postfix.
	DocIterator const & inlineCompletionPos() const;
	/// make sure inline completion position is OK
	void resetInlineCompletionPos();
	/// set the inline completion postfix and its position in the buffer.
	/// Updates the updateFlags in \c cur.
	void setInlineCompletion(Cursor const & cur, DocIterator const & pos,
		docstring const & completion, size_t uniqueChars = 0);

	/// translate and insert a character, using the correct keymap.
	void translateAndInsert(char_type c, Text * t, Cursor & cur);

	/// \return true if we've made a decision
	bool getStatus(FuncRequest const & cmd, FuncStatus & flag);
	/// execute the given function.
	void dispatch(FuncRequest const & cmd, DispatchResult & dr);

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
	/// This will also open all relevant collapsible insets.
	void setCursor(DocIterator const &);
	/// set the selection up to dit.
	void setCursorSelectionTo(DocIterator const & dit);
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

	/// selects the item at cursor if its paragraph is empty.
	bool selectIfEmpty(DocIterator & cur);

	/// update the internal \c ViewMetricsInfo.
	void updateMetrics();

	// this is the "nodraw" drawing stage: only set the positions of the
	// insets in metrics cache.
	void updatePosCache();

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
	Point getPos(DocIterator const & dit) const;
	/// is the paragraph of the cursor visible ?
	bool paragraphVisible(DocIterator const & dit) const;
	/// is the cursor currently visible in the view
	bool cursorInView(Point const & p, int h) const;
	/// get the position and height of the caret
	void caretPosAndHeight(Point & p, int & h) const;

	///
	void draw(frontend::Painter & pain, bool paint_caret);

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
	void showDialog(std::string const & name, std::string const & data,
		Inset * inset = 0);

	/// This signal is emitted when some dialogs needs to be updated.
	void updateDialog(std::string const & name, std::string const & data);

	///
	void setGuiDelegate(frontend::GuiBufferViewDelegate *);

	///
	docstring contentsOfPlaintextFile(support::FileName const & f);
	// Insert plain text file (if filename is empty, prompt for one)
	void insertPlaintextFile(support::FileName const & f, bool asParagraph);
	///
	void insertLyXFile(support::FileName const & f);
	/// save temporary bookmark for jump back navigation
	void bookmarkEditPosition();
	/// Find and return the inset associated with given dialog name.
	Inset * editedInset(std::string const & name) const;
	/// Associate an inset associated with given dialog name.
	void editInset(std::string const & name, Inset * inset);
	///
	void clearLastInset(Inset * inset) const;
	/// Is the mouse hovering a clickable inset or element?
	bool clickableInset() const;
	///
	void makeDocumentClass();

private:
	/// noncopyable
	BufferView(BufferView const &);
	void operator=(BufferView const &);

	/// the position relative to (0, baseline) of outermost paragraph
	Point coordOffset(DocIterator const & dit) const;
	/// Update current paragraph metrics.
	/// \return true if no further update is needed.
	bool singleParUpdate();
	/// do the work for the public updateMetrics()
	void updateMetrics(Update::flags & update_flags);

	// Set the row on which the cursor lives.
	void setCurrentRowSlice(CursorSlice const & rowSlice);

	// Check whether the row where the cursor lives needs to be scrolled.
	// Update the drawing strategy if needed.
	void checkCursorScrollOffset();

	/// The minimal size of the document that is visible. Used
	/// when it is allowed to scroll below the document.
	int minVisiblePart();

	/// Search recursively for the innermost inset that covers (x, y) position.
	/// \retval 0 if no inset is found.
	Inset const * getCoveringInset(
		Text const & text, //< The Text where we start searching.
		int x, //< x-coordinate on screen
		int y  //< y-coordinate on screen
		) const;

	/// Update the hovering status of the insets. This is called when
	/// either the screen is updated or when the buffer has scolled.
	void updateHoveredInset() const;

	///
	void updateDocumentClass(DocumentClassConstPtr olddc);
	///
	int width_;
	///
	int height_;
	///
	bool full_screen_;
	///
	Buffer & buffer_;

	struct Private;
	Private * const d;
};

/// some space for drawing the 'nested' markers (in pixel)
inline int nestMargin() { return 15; }

/// margin for changebar
inline int changebarMargin() { return 12; }

} // namespace lyx

#endif // BUFFERVIEW_H
