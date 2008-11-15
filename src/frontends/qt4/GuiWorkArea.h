// -*- C++ -*-
/**
 * \file GuiWorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORKAREA_H
#define WORKAREA_H

#include "frontends/WorkArea.h"

#include "DocIterator.h"
#include "FuncRequest.h"
#include "qt_helpers.h"
#include "support/docstring.h"
#include "support/Timeout.h"

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>

class QContextMenuEvent;
class QDragEnterEvent;
class QDropEvent;
class QKeyEvent;
class QWheelEvent;
class QPaintEvent;
class QWidget;

#ifdef CursorShape
#undef CursorShape
#endif

namespace lyx {

class Buffer;

namespace frontend {

class GuiCompleter;
class GuiView;
class GuiWorkArea;

/// for emulating triple click
class DoubleClick {
public:
	///
	DoubleClick() : state(Qt::NoButton), active(false) {}
	///
	DoubleClick(QMouseEvent * e) : state(e->button()), active(true) {}
	///
	bool operator==(QMouseEvent const & e) { return state == e.button(); }
	///
public:
	///
	Qt::MouseButton state;
	///
	bool active;
};

/** Qt only emits mouse events when the mouse is being moved, but
 *  we want to generate 'pseudo' mouse events when the mouse button is
 *  pressed and the mouse cursor is below the bottom, or above the top
 *  of the work area. In this way, we'll be able to continue scrolling
 *  (and selecting) the text.
 *
 *  This class stores all the parameters needed to make this happen.
 */
class SyntheticMouseEvent
{
public:
	SyntheticMouseEvent();

	FuncRequest cmd;
	Timeout timeout;
	bool restart_timeout;
	int x_old;
	int y_old;
	double scrollbar_value_old;
};


/**
 * Implementation of the work area (buffer view GUI)
*/
class CursorWidget;

class GuiWorkArea : public QAbstractScrollArea, public WorkArea
{
	Q_OBJECT

public:
	///
	GuiWorkArea(QWidget *);
	///
	GuiWorkArea(Buffer & buffer, GuiView & gv);
	///
	~GuiWorkArea();

	///
	void setBuffer(Buffer &);
	///
	void setGuiView(GuiView &);
	/// Dummy methods for Designer.
	void setWidgetResizable(bool) {}
	void setWidget(QWidget *) {}
	///
	void setFullScreen(bool full_screen);
	/// is LyXView in fullscreen mode?
	bool isFullScreen();
	///
	void scheduleRedraw() { schedule_redraw_ = true; }
	///
	BufferView & bufferView();
	///
	BufferView const & bufferView() const;
	///
	void redraw();
	///
	void stopBlinkingCursor();
	///
	void startBlinkingCursor();
	/// Process Key pressed event.
	/// This needs to be public because it is accessed externally by GuiView.
	void processKeySym(KeySymbol const & key, KeyModifier mod);
	///
	void resizeBufferView();

	bool isInDialog() {
		return dialogMode_;
	}

	///
	GuiCompleter & completer() { return *completer_; }

	/// Return true if dialogMode is set
	bool& dialogMode() { return dialogMode_; }

	/// Return the GuiView this workArea belongs to
	GuiView const & view() const { return *lyx_view_; }
	GuiView & view() { return *lyx_view_; }

Q_SIGNALS:
	///
	void titleChanged(GuiWorkArea *);

private Q_SLOTS:
	/// Scroll the BufferView.
	/**
	  * This is a slot for the valueChanged() signal of the vertical scrollbar.
	  * \p value value of the scrollbar.
	*/
	void scrollTo(int value);
	/// timer to limit triple clicks
	void doubleClickTimeout();
	/// toggle the cursor's visibility
	void toggleCursor();
	/// close this work area.
	/// Slot for Buffer::closing signal.
	void close();
	/// Slot to restore proper scrollbar behaviour.
	void fixVerticalScrollBar();

private:
	friend class GuiCompleter;
	///
	void init();

	/// update the passed area.
	void update(int x, int y, int w, int h);
	///
	void updateScreen();

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h,
		bool l_shape, bool rtl, bool completable);

	/// hide the cursor
	virtual void removeCursor();

	/// This function is called when the buffer readonly status change.
	void setReadOnly(bool);

	/// Update window titles of all users.
	void updateWindowTitle();
	///
	bool event(QEvent *);
	///
	void contextMenuEvent(QContextMenuEvent *);
	///
	void focusInEvent(QFocusEvent *);
	///
	void focusOutEvent(QFocusEvent *);
	/// repaint part of the widget
	void paintEvent(QPaintEvent * ev);
	/// widget has been resized
	void resizeEvent(QResizeEvent * ev);
	/// mouse button press
	void mousePressEvent(QMouseEvent * ev);
	/// mouse button release
	void mouseReleaseEvent(QMouseEvent * ev);
	/// mouse double click of button
	void mouseDoubleClickEvent(QMouseEvent * ev);
	/// mouse motion
	void mouseMoveEvent(QMouseEvent * ev);
	/// wheel event
	void wheelEvent(QWheelEvent * ev);
	/// key press
	void keyPressEvent(QKeyEvent * ev);
	/// IM events
	void inputMethodEvent(QInputMethodEvent * ev);
	/// IM query
	QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();
	///
	void dispatch(FuncRequest const & cmd0, KeyModifier = NoModifier);
	/// hide the visible cursor, if it is visible
	void hideCursor();
	/// show the cursor if it is not visible
	void showCursor();
	///
	void updateScrollbar();

	///
	BufferView * buffer_view_;
	///
	GuiView * lyx_view_;
	/// is the cursor currently displayed
	bool cursor_visible_;

	///
	QTimer cursor_timeout_;
	///
	SyntheticMouseEvent synthetic_mouse_event_;
	///
	DoubleClick dc_event_;

	///
	CursorWidget * cursor_;
	///
	QPixmap screen_;
	///
	bool need_resize_;
	///
	bool schedule_redraw_;
	///
	int preedit_lines_;

	///
	GuiCompleter * completer_;

	/// Special mode in which Esc and Enter (with or without Shift)
	/// are ignored
	bool dialogMode_;
}; // GuiWorkArea


/// A tabbed set of GuiWorkAreas.
class TabWorkArea : public QTabWidget
{
	Q_OBJECT
public:
	TabWorkArea(QWidget * parent = 0);

	///
	void setFullScreen(bool full_screen);
	void showBar(bool show);
	void closeAll();
	bool setCurrentWorkArea(GuiWorkArea *);
	GuiWorkArea * addWorkArea(Buffer & buffer, GuiView & view);
	bool removeWorkArea(GuiWorkArea *);
	GuiWorkArea * currentWorkArea();
	GuiWorkArea * workArea(Buffer & buffer);

Q_SIGNALS:
	///
	void currentWorkAreaChanged(GuiWorkArea *);
	///
	void lastWorkAreaRemoved();

public Q_SLOTS:
	/// close current buffer, or the one given by \c clicked_tab_
	void closeCurrentBuffer();
	/// close current tab, or the one given by \c clicked_tab_
	void closeCurrentTab();
	///
	void updateTabTexts();
	
private Q_SLOTS:
	///
	void on_currentTabChanged(int index);
	///
	void showContextMenu(const QPoint & pos);
	///
	void moveTab(int fromIndex, int toIndex);

private:
	int clicked_tab_;
}; // TabWorkArea


class DragTabBar : public QTabBar
{
	Q_OBJECT
public:
	///
	DragTabBar(QWidget * parent = 0);

#if QT_VERSION < 0x040300
	///
	int tabAt(QPoint const & position) const;
#endif

protected:
	///
	void mousePressEvent(QMouseEvent * event);
	///
	void mouseMoveEvent(QMouseEvent * event);
	///
	void dragEnterEvent(QDragEnterEvent * event);
	///
	void dropEvent(QDropEvent * event);

private:
	///
	QPoint dragStartPos_;
	///
	int dragCurrentIndex_;

Q_SIGNALS:
	///
	void tabMoveRequested(int fromIndex, int toIndex);
};

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
