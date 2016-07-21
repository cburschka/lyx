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

#include <QAbstractScrollArea>
#include <QTabBar>
#include <QTabWidget>

class QDragEnterEvent;
class QDropEvent;
class QToolButton;
class QWidget;

#ifdef CursorShape
#undef CursorShape
#endif

namespace lyx {

class Buffer;

namespace frontend {

class GuiCompleter;
class GuiView;

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
	void init();
	///
	void setBuffer(Buffer &);
	///
	void setGuiView(GuiView &);
	///
	void setFullScreen(bool full_screen);
	/// is GuiView in fullscreen mode?
	bool isFullScreen() const;
	///
	void scheduleRedraw();
	///
	BufferView & bufferView();
	///
	BufferView const & bufferView() const;
	///
	void redraw(bool update_metrics);

	/// return true if the key is part of a shortcut
	bool queryKeySym(KeySymbol const & key, KeyModifier mod) const;
	/// Process Key pressed event.
	/// This needs to be public because it is accessed externally by GuiView.
	void processKeySym(KeySymbol const & key, KeyModifier mod);

	bool inDialogMode() const;
	void setDialogMode(bool mode);

	///
	GuiCompleter & completer();

	Qt::CursorShape cursorShape() const;

	/// Return the GuiView this workArea belongs to
	GuiView const & view() const;
	GuiView & view();

	/// Current ratio between physical pixels and device-independent pixels
	double pixelRatio() const;

public Q_SLOTS:
	///
	void stopBlinkingCursor();
	///
	void startBlinkingCursor();

Q_SIGNALS:
	///
	void titleChanged(GuiWorkArea *);
	///
	void busy(bool);

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
	/// key press event. It also knows how to handle ShortcutOverride events to
	/// avoid code duplication.
	void keyPressEvent(QKeyEvent * ev);
	/// IM events
	void inputMethodEvent(QInputMethodEvent * ev);
	/// IM query
	QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();

	friend class GuiCompleter;
	struct Private;
	Private * const d;
}; // GuiWorkArea


class EmbeddedWorkArea : public GuiWorkArea
{
	Q_OBJECT
public:
	///
	EmbeddedWorkArea(QWidget *);
	~EmbeddedWorkArea();

	/// Dummy methods for Designer.
	void setWidgetResizable(bool) {}
	void setWidget(QWidget *) {}

	QSize sizeHint () const;
	///
	void disable();

protected:
	///
	void closeEvent(QCloseEvent * ev);
	///
	void hideEvent(QHideEvent *ev);

private:
	/// Embedded Buffer.
	Buffer * buffer_;
}; // EmbeddedWorkArea


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
	GuiWorkArea * workArea(int index);
	void paintEvent(QPaintEvent *);

Q_SIGNALS:
	///
	void currentWorkAreaChanged(GuiWorkArea *);
	///
	void lastWorkAreaRemoved();

public Q_SLOTS:
	/// close current buffer, or the one given by \c clicked_tab_
	void closeCurrentBuffer();
	/// hide current tab, or the one given by \c clicked_tab_
	void hideCurrentTab();
	/// close the tab given by \c index
	void closeTab(int index);
	///
	void moveTab(int fromIndex, int toIndex);
	///
	void updateTabTexts();
	
private Q_SLOTS:
	///
	void on_currentTabChanged(int index);
	///
	void showContextMenu(const QPoint & pos);
	///
	void mouseDoubleClickEvent(QMouseEvent * event);

private:
	/// true if position is a tab (rather than the blank space in tab bar)
	bool posIsTab(QPoint position);

	int clicked_tab_;
	///
	QToolButton * closeBufferButton;
}; // TabWorkArea


class DragTabBar : public QTabBar
{
	Q_OBJECT
public:
	///
	DragTabBar(QWidget * parent = 0);

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

Q_SIGNALS:
	///
	void tabMoveRequested(int fromIndex, int toIndex);
};

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
