/**
 * \file GuiWorkArea.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiWorkArea.h"

#include "ColorCache.h"
#include "FontLoader.h"
#include "Menus.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "Font.h"
#include "FuncRequest.h"
#include "GuiApplication.h"
#include "GuiCompleter.h"
#include "GuiKeySymbol.h"
#include "GuiPainter.h"
#include "GuiView.h"
#include "KeySymbol.h"
#include "Language.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "qt_helpers.h"
#include "Text.h"
#include "version.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/FileName.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/WorkAreaManager.h"

#include <QContextMenuEvent>
#include <QInputContext>
#include <QHelpEvent>
#ifdef Q_WS_MACX
#include <QMacStyle>
#endif
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QPixmapCache>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QMenuBar>

#include <boost/bind.hpp>

#ifdef Q_WS_X11
#include <QX11Info>
extern "C" int XEventsQueued(Display *display, int mode);
#endif

#ifdef Q_WS_WIN
int const CursorWidth = 2;
#else
int const CursorWidth = 1;
#endif
int const TabIndicatorWidth = 3;

#undef KeyPress
#undef NoModifier 

using namespace std;
using namespace lyx::support;

namespace lyx {


/// return the LyX mouse button state from Qt's
static mouse_button::state q_button_state(Qt::MouseButton button)
{
	mouse_button::state b = mouse_button::none;
	switch (button) {
		case Qt::LeftButton:
			b = mouse_button::button1;
			break;
		case Qt::MidButton:
			b = mouse_button::button2;
			break;
		case Qt::RightButton:
			b = mouse_button::button3;
			break;
		default:
			break;
	}
	return b;
}


/// return the LyX mouse button state from Qt's
mouse_button::state q_motion_state(Qt::MouseButtons state)
{
	mouse_button::state b = mouse_button::none;
	if (state & Qt::LeftButton)
		b |= mouse_button::button1;
	if (state & Qt::MidButton)
		b |= mouse_button::button2;
	if (state & Qt::RightButton)
		b |= mouse_button::button3;
	return b;
}


namespace frontend {

class CursorWidget {
public:
	CursorWidget() {}

	void draw(QPainter & painter)
	{
		if (!show_ || !rect_.isValid())
			return;
		
		int y = rect_.top();
		int l = x_ - rect_.left();
		int r = rect_.right() - x_;
		int bot = rect_.bottom();

		// draw vertica linel
		painter.fillRect(x_, y, CursorWidth, rect_.height(), color_);
		
		// draw RTL/LTR indication
		painter.setPen(color_);
		if (l_shape_) {
			if (rtl_)
				painter.drawLine(x_, bot, x_ - l, bot);
			else
				painter.drawLine(x_, bot, x_ + CursorWidth + r, bot);
		}
		
		// draw completion triangle
		if (completable_) {
			int m = y + rect_.height() / 2;
			int d = TabIndicatorWidth - 1;
			if (rtl_) {
				painter.drawLine(x_ - 1, m - d, x_ - 1 - d, m);
				painter.drawLine(x_ - 1, m + d, x_ - 1 - d, m);
			} else {
				painter.drawLine(x_ + CursorWidth, m - d, x_ + CursorWidth + d, m);
				painter.drawLine(x_ + CursorWidth, m + d, x_ + CursorWidth + d, m);
			}
		}
	}

	void update(int x, int y, int h, bool l_shape,
		bool rtl, bool completable)
	{
		color_ = guiApp->colorCache().get(Color_cursor);
		l_shape_ = l_shape;
		rtl_ = rtl;
		completable_ = completable;
		x_ = x;
		
		// extension to left and right
		int l = 0;
		int r = 0;

		// RTL/LTR indication
		if (l_shape_) {
			if (rtl)
				l += h / 3;
			else
				r += h / 3;
		}
		
		// completion triangle
		if (completable_) {
			if (rtl)
				l = max(l, TabIndicatorWidth);
			else
				r = max(r, TabIndicatorWidth);
		}

		// compute overall rectangle
		rect_ = QRect(x - l, y, CursorWidth + r + l, h);
	}

	void show(bool set_show = true) { show_ = set_show; }
	void hide() { show_ = false; }

	QRect const & rect() { return rect_; }

private:
	/// cursor is in RTL or LTR text
	bool rtl_;
	/// indication for RTL or LTR
	bool l_shape_;
	/// triangle to show that a completion is available
	bool completable_;
	///
	bool show_;
	///
	QColor color_;
	/// rectangle, possibly with l_shape and completion triangle
	QRect rect_;
	/// x position (were the vertical line is drawn)
	int x_;
};


// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true),
	  x_old(-1), y_old(-1), scrollbar_value_old(-1.0)
{}



GuiWorkArea::GuiWorkArea(Buffer & buffer, GuiView & lv)
	: buffer_view_(new BufferView(buffer)), lyx_view_(&lv),
	cursor_visible_(false),
	need_resize_(false), schedule_redraw_(false),
	preedit_lines_(1), completer_(new GuiCompleter(this))
{
	buffer.workAreaManager().add(this);
	// Setup the signals
	connect(&cursor_timeout_, SIGNAL(timeout()),
		this, SLOT(toggleCursor()));
	
	int const time = QApplication::cursorFlashTime() / 2;
	if (time > 0) {
		cursor_timeout_.setInterval(time);
		cursor_timeout_.start();
	} else {
		// let's initialize this just to be safe
		cursor_timeout_.setInterval(500);
	}

	screen_ = QPixmap(viewport()->width(), viewport()->height());
	cursor_ = new frontend::CursorWidget();
	cursor_->hide();

	// HACK: Prevents an additional redraw when the scrollbar pops up
	// which regularily happens on documents with more than one page.
	// The policy  should be set to "Qt::ScrollBarAsNeeded" soon.
	// Since we have no geometry information yet, we assume that
	// a document needs a scrollbar if there is more then four
	// paragraph in the outermost text.
	if (buffer.text().paragraphs().size() > 4)
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	QTimer::singleShot(50, this, SLOT(fixVerticalScrollBar()));


	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptDrops(true);
	setMouseTracking(true);
	setMinimumSize(100, 70);
#ifdef Q_WS_MACX
	setFrameStyle(QFrame::NoFrame);	
#else
	setFrameStyle(QFrame::Box);
#endif
	updateWindowTitle();

	viewport()->setAutoFillBackground(false);
	// We don't need double-buffering nor SystemBackground on
	// the viewport because we have our own backing pixmap.
	viewport()->setAttribute(Qt::WA_NoSystemBackground);

	setFocusPolicy(Qt::WheelFocus);

	viewport()->setCursor(Qt::IBeamCursor);

	synthetic_mouse_event_.timeout.timeout.connect(
		boost::bind(&GuiWorkArea::generateSyntheticMouseEvent,
					this));

	// Initialize the vertical Scroll Bar
	QObject::connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		this, SLOT(scrollTo(int)));

	LYXERR(Debug::GUI, "viewport width: " << viewport()->width()
		<< "  viewport height: " << viewport()->height());

	// Enables input methods for asian languages.
	// Must be set when creating custom text editing widgets.
	setAttribute(Qt::WA_InputMethodEnabled, true);
}


GuiWorkArea::~GuiWorkArea()
{
	buffer_view_->buffer().workAreaManager().remove(this);
	delete buffer_view_;
	delete cursor_;
}


void GuiWorkArea::fixVerticalScrollBar()
{
	if (!isFullScreen())
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


void GuiWorkArea::close()
{
	lyx_view_->removeWorkArea(this);
}


void GuiWorkArea::setFullScreen(bool full_screen)
{
	buffer_view_->setFullScreen(full_screen);
	if (full_screen) {
		setFrameStyle(QFrame::NoFrame);
		if (lyxrc.full_screen_scrollbar)
			setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
#ifdef Q_WS_MACX
		setFrameStyle(QFrame::NoFrame);	
#else
		setFrameStyle(QFrame::Box);
#endif
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}
}


BufferView & GuiWorkArea::bufferView()
{
	return *buffer_view_;
}


BufferView const & GuiWorkArea::bufferView() const
{
	return *buffer_view_;
}


void GuiWorkArea::stopBlinkingCursor()
{
	cursor_timeout_.stop();
	hideCursor();
}


void GuiWorkArea::startBlinkingCursor()
{
	showCursor();
	//we're not supposed to cache this value.
	int const time = QApplication::cursorFlashTime() / 2;
	if (time <= 0)
		return;
	cursor_timeout_.setInterval(time);
	cursor_timeout_.start();
}


void GuiWorkArea::redraw()
{
	if (!isVisible())
		// No need to redraw in this case.
		return;

	// No need to do anything if this is the current view. The BufferView
	// metrics are already up to date.
	if (lyx_view_ != guiApp->currentView()
		|| lyx_view_->currentWorkArea() != this) {
		// FIXME: it would be nice to optimize for the off-screen case.
		buffer_view_->updateMetrics();
		buffer_view_->cursor().fixIfBroken();
	}

	// update cursor position, because otherwise it has to wait until
	// the blinking interval is over
	if (cursor_visible_) {
		hideCursor();
		showCursor();
	}
	
	LYXERR(Debug::WORKAREA, "WorkArea::redraw screen");
	updateScreen();
	update(0, 0, viewport()->width(), viewport()->height());

	/// \warning: scrollbar updating *must* be done after the BufferView is drawn
	/// because \c BufferView::updateScrollbar() is called in \c BufferView::draw().
	updateScrollbar();
	lyx_view_->updateStatusBar();

	if (lyxerr.debugging(Debug::WORKAREA))
		buffer_view_->coordCache().dump();
}


void GuiWorkArea::processKeySym(KeySymbol const & key, KeyModifier mod)
{
	if (lyx_view_->isFullScreen() && lyx_view_->menuBar()->isVisible()) {
		// FIXME HACK: we should not have to do this here. See related comment
		// in GuiView::event() (QEvent::ShortcutOverride)
		lyx_view_->menuBar()->hide();
	}

	// In order to avoid bad surprise in the middle of an operation,
	// we better stop the blinking cursor...
	// the cursor gets restarted in GuiView::restartCursor()
	stopBlinkingCursor();

	theLyXFunc().setLyXView(lyx_view_);
	theLyXFunc().processKeySym(key, mod);
}


void GuiWorkArea::dispatch(FuncRequest const & cmd0, KeyModifier mod)
{
	// Handle drag&drop
	if (cmd0.action == LFUN_FILE_OPEN) {
		lyx_view_->dispatch(cmd0);
		return;
	}

	theLyXFunc().setLyXView(lyx_view_);

	FuncRequest cmd;

	if (cmd0.action == LFUN_MOUSE_PRESS) {
		if (mod == ShiftModifier)
			cmd = FuncRequest(cmd0, "region-select");
		else if (mod == ControlModifier)
			cmd = FuncRequest(cmd0, "paragraph-select");
		else
			cmd = cmd0;
	}
	else
		cmd = cmd0;

	bool const notJustMovingTheMouse = 
		cmd.action != LFUN_MOUSE_MOTION || cmd.button() != mouse_button::none;
	
	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking cursor.
	if (notJustMovingTheMouse)
		stopBlinkingCursor();

	buffer_view_->mouseEventDispatch(cmd);

	// Skip these when selecting
	if (cmd.action != LFUN_MOUSE_MOTION) {
		completer_->updateVisibility(false, false);
		lyx_view_->updateDialogs();
		lyx_view_->updateStatusBar();
	}

	// GUI tweaks except with mouse motion with no button pressed.
	if (notJustMovingTheMouse) {
		// Slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		lyx_view_->clearMessage();

		// Show the cursor immediately after any operation
		startBlinkingCursor();
	}
}


void GuiWorkArea::resizeBufferView()
{
	// WARNING: Please don't put any code that will trigger a repaint here!
	// We are already inside a paint event.
	lyx_view_->setBusy(true);
	buffer_view_->resize(viewport()->width(), viewport()->height());
	updateScreen();

	// Update scrollbars which might have changed due different
	// BufferView dimension. This is especially important when the 
	// BufferView goes from zero-size to the real-size for the first time,
	// as the scrollbar paramters are then set for the first time.
	updateScrollbar();
	
	lyx_view_->updateLayoutList();
	lyx_view_->setBusy(false);
	need_resize_ = false;
}


void GuiWorkArea::showCursor()
{
	if (cursor_visible_)
		return;

	// RTL or not RTL
	bool l_shape = false;
	Font const & realfont = buffer_view_->cursor().real_current_font;
	BufferParams const & bp = buffer_view_->buffer().params();
	bool const samelang = realfont.language() == bp.language;
	bool const isrtl = realfont.isVisibleRightToLeft();

	if (!samelang || isrtl != bp.language->rightToLeft())
		l_shape = true;

	// The ERT language hack needs fixing up
	if (realfont.language() == latex_language)
		l_shape = false;

	Font const font = buffer_view_->cursor().getFont();
	FontMetrics const & fm = theFontMetrics(font);
	int const asc = fm.maxAscent();
	int const des = fm.maxDescent();
	int h = asc + des;
	int x = 0;
	int y = 0;
	Cursor & cur = buffer_view_->cursor();
	cur.getPos(x, y);
	y -= asc;

	// if it doesn't touch the screen, don't try to show it
	bool cursorInView = true;
	if (y + h < 0 || y >= viewport()->height())
		cursorInView = false;

	// show cursor on screen
	bool completable = cur.inset().showCompletionCursor()
		&& completer_->completionAvailable()
		&& !completer_->popupVisible()
		&& !completer_->inlineVisible();
	if (cursorInView) {
		cursor_visible_ = true;
		showCursor(x, y, h, l_shape, isrtl, completable);
	}
}


void GuiWorkArea::hideCursor()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	removeCursor();
}


void GuiWorkArea::toggleCursor()
{
	if (cursor_visible_)
		hideCursor();
	else
		showCursor();
}


void GuiWorkArea::updateScrollbar()
{
	ScrollbarParameters const & scroll_ = buffer_view_->scrollbarParameters();
	// WARNING: don't touch at the scrollbar value like this:
	//   verticalScrollBar()->setValue(scroll_.position);
	// because this would cause a recursive signal/slot calling with
	// GuiWorkArea::scrollTo
	verticalScrollBar()->setRange(scroll_.min, scroll_.max);
	verticalScrollBar()->setPageStep(scroll_.page_step);
	verticalScrollBar()->setSingleStep(scroll_.single_step);
	verticalScrollBar()->setSliderPosition(scroll_.position);
}


void GuiWorkArea::scrollTo(int value)
{
	stopBlinkingCursor();
	buffer_view_->scrollDocView(value);

	if (lyxrc.cursor_follows_scrollbar) {
		buffer_view_->setCursorFromScrollbar();
		lyx_view_->updateLayoutList();
	}
	// Show the cursor immediately after any operation.
	startBlinkingCursor();
	QApplication::syncX();
}


bool GuiWorkArea::event(QEvent * e)
{
	switch (e->type()) {
	case QEvent::ToolTip: {
		QHelpEvent * helpEvent = static_cast<QHelpEvent *>(e);
		if (lyxrc.use_tooltip) {
			QPoint pos = helpEvent->pos();
			if (pos.x() < viewport()->width()) {
				QString s = toqstr(buffer_view_->toolTip(pos.x(), pos.y()));
				QToolTip::showText(helpEvent->globalPos(), s);
			}
			else
				QToolTip::hideText();
		}
		// Don't forget to accept the event!
		e->accept();
		return true;
	}

	case QEvent::ShortcutOverride: {
		// We catch this event in order to catch the Tab or Shift+Tab key press
		// which are otherwise reserved to focus switching between controls
		// within a dialog.
		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		if ((ke->key() != Qt::Key_Tab && ke->key() != Qt::Key_Backtab)
			|| ke->modifiers() & Qt::ControlModifier)
			return QAbstractScrollArea::event(e);
		keyPressEvent(ke);
		return true;
	}

	default:
		return QAbstractScrollArea::event(e);
	}
	return false;
}


void GuiWorkArea::contextMenuEvent(QContextMenuEvent * e)
{
	QPoint pos = e->pos();
	docstring name = buffer_view_->contextMenu(pos.x(), pos.y());
	if (name.empty()) {
		QAbstractScrollArea::contextMenuEvent(e);
		return;
	}
	QMenu * menu = guiApp->menus().menu(toqstr(name), *lyx_view_);
	if (!menu) {
		QAbstractScrollArea::contextMenuEvent(e);
		return;
	}
	// Position the menu to the right.
	// FIXME: menu position should be different for RTL text.
	menu->exec(e->globalPos());
	e->accept();
}


void GuiWorkArea::focusInEvent(QFocusEvent * e)
{
	if (lyx_view_->currentWorkArea() != this)
		lyx_view_->setCurrentWorkArea(this);

	startBlinkingCursor();
	QAbstractScrollArea::focusInEvent(e);
}


void GuiWorkArea::focusOutEvent(QFocusEvent * e)
{
	stopBlinkingCursor();
	QAbstractScrollArea::focusOutEvent(e);
}


void GuiWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (dc_event_.active && dc_event_ == *e) {
		dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE, e->x(), e->y(),
			q_button_state(e->button()));
		dispatch(cmd);
		e->accept();
		return;
	}

	inputContext()->reset();

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
		q_button_state(e->button()));
	dispatch(cmd, q_key_state(e->modifiers()));
	e->accept();
}


void GuiWorkArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (synthetic_mouse_event_.timeout.running())
		synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			      q_button_state(e->button()));
	dispatch(cmd);
	e->accept();
}


void GuiWorkArea::mouseMoveEvent(QMouseEvent * e)
{
	// we kill the triple click if we move
	doubleClickTimeout();
	FuncRequest cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
		q_motion_state(e->buttons()));

	e->accept();

	// If we're above or below the work area...
	if (e->y() <= 20 || e->y() >= viewport()->height() - 20) {
		// Make sure only a synthetic event can cause a page scroll,
		// so they come at a steady rate:
		if (e->y() <= 20)
			// _Force_ a scroll up:
			cmd.y = -40;
		else
			cmd.y = viewport()->height();
		// Store the event, to be handled when the timeout expires.
		synthetic_mouse_event_.cmd = cmd;

		if (synthetic_mouse_event_.timeout.running())
			// Discard the event. Note that it _may_ be handled
			// when the timeout expires if
			// synthetic_mouse_event_.cmd has not been overwritten.
			// Ie, when the timeout expires, we handle the
			// most recent event but discard all others that
			// occurred after the one used to start the timeout
			// in the first place.
			return;

		synthetic_mouse_event_.restart_timeout = true;
		synthetic_mouse_event_.timeout.start();
		// Fall through to handle this event...

	} else if (synthetic_mouse_event_.timeout.running()) {
		// Store the event, to be possibly handled when the timeout
		// expires.
		// Once the timeout has expired, normal control is returned
		// to mouseMoveEvent (restart_timeout = false).
		// This results in a much smoother 'feel' when moving the
		// mouse back into the work area.
		synthetic_mouse_event_.cmd = cmd;
		synthetic_mouse_event_.restart_timeout = false;
		return;
	}

	// Has anything changed on-screen since the last QMouseEvent
	// was received?
	double const scrollbar_value = verticalScrollBar()->value();
	if (e->x() == synthetic_mouse_event_.x_old
		&& e->y() == synthetic_mouse_event_.y_old
		&& scrollbar_value == synthetic_mouse_event_.scrollbar_value_old) {
		// Nothing changed on-screen since the last QMouseEvent.
		return;
	}

	// Yes something has changed. Store the params used to check this.
	synthetic_mouse_event_.x_old = e->x();
	synthetic_mouse_event_.y_old = e->y();
	synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

	// ... and dispatch the event to the LyX core.
	dispatch(cmd);
}


void GuiWorkArea::wheelEvent(QWheelEvent * ev)
{
	// Wheel rotation by one notch results in a delta() of 120 (see
	// documentation of QWheelEvent)
	int delta = ev->delta() / 120;
	if (ev->modifiers() & Qt::ControlModifier) {
		lyxrc.zoom -= 5 * delta;
		if (lyxrc.zoom < 10)
			lyxrc.zoom = 10;
		// The global QPixmapCache is used in GuiPainter to cache text
		// painting so we must reset it.
		QPixmapCache::clear();
		guiApp->fontLoader().update();
		lyx::dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
	} else {
		double const lines = qApp->wheelScrollLines()
			* lyxrc.mouse_wheel_speed * delta;
		LYXERR(Debug::SCROLLING, "wheelScrollLines = " << qApp->wheelScrollLines()
			<< " delta = " << ev->delta() << " lines = " << lines);
		verticalScrollBar()->setValue(verticalScrollBar()->value() -
			int(lines *  verticalScrollBar()->singleStep()));
	}
	ev->accept();
}


void GuiWorkArea::generateSyntheticMouseEvent()
{
	// Set things off to generate the _next_ 'pseudo' event.
	if (synthetic_mouse_event_.restart_timeout)
		synthetic_mouse_event_.timeout.start();

	// Has anything changed on-screen since the last timeout signal
	// was received?
	double const scrollbar_value = verticalScrollBar()->value();
	if (scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		dispatch(synthetic_mouse_event_.cmd);
	}
}


void GuiWorkArea::keyPressEvent(QKeyEvent * ev)
{
	// intercept some keys if completion popup is visible
	if (completer_->popupVisible()) {
		switch (ev->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			completer_->activate();
			ev->accept();
			return;
		}
	}
	
	// intercept keys for the completion
	if (ev->key() == Qt::Key_Tab) {
		completer_->tab();
		ev->accept();
		return;
	} 

	if (completer_->popupVisible() && ev->key() == Qt::Key_Escape) {
		completer_->hidePopup();
		ev->accept();
		return;
	}

	if (completer_->inlineVisible() && ev->key() == Qt::Key_Escape) {
		completer_->hideInline();
		ev->accept();
		return;
	}

	// do nothing if there are other events
	// (the auto repeated events come too fast)
	// \todo FIXME: remove hard coded Qt keys, process the key binding
#ifdef Q_WS_X11
	if (XEventsQueued(QX11Info::display(), 0) > 1 && ev->isAutoRepeat() 
			&& (Qt::Key_PageDown || Qt::Key_PageUp)) {
		LYXERR(Debug::KEY, "system is busy: scroll key event ignored");
		ev->ignore();
		return;
	}
#endif

	LYXERR(Debug::KEY, " count: " << ev->count() << " text: " << ev->text()
		<< " isAutoRepeat: " << ev->isAutoRepeat() << " key: " << ev->key());

	KeySymbol sym;
	setKeySymbol(&sym, ev);
	processKeySym(sym, q_key_state(ev->modifiers()));
	ev->accept();
}


void GuiWorkArea::doubleClickTimeout()
{
	dc_event_.active = false;
}


void GuiWorkArea::mouseDoubleClickEvent(QMouseEvent * ev)
{
	dc_event_ = DoubleClick(ev);
	QTimer::singleShot(QApplication::doubleClickInterval(), this,
			   SLOT(doubleClickTimeout()));
	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
			ev->x(), ev->y(),
			q_button_state(ev->button()));
	dispatch(cmd);
	ev->accept();
}


void GuiWorkArea::resizeEvent(QResizeEvent * ev)
{
	QAbstractScrollArea::resizeEvent(ev);
	need_resize_ = true;
	ev->accept();
}


void GuiWorkArea::update(int x, int y, int w, int h)
{
	viewport()->repaint(x, y, w, h);
}


void GuiWorkArea::paintEvent(QPaintEvent * ev)
{
	QRect const rc = ev->rect();
	// LYXERR(Debug::PAINTING, "paintEvent begin: x: " << rc.x()
	//	<< " y: " << rc.y() << " w: " << rc.width() << " h: " << rc.height());

	if (need_resize_) {
		screen_ = QPixmap(viewport()->width(), viewport()->height());
		resizeBufferView();
		hideCursor();
		showCursor();
	}

	QPainter pain(viewport());
	pain.drawPixmap(rc, screen_, rc);
	cursor_->draw(pain);
	ev->accept();
}


void GuiWorkArea::updateScreen()
{
	GuiPainter pain(&screen_);
	buffer_view_->draw(pain);
}


void GuiWorkArea::showCursor(int x, int y, int h,
	bool l_shape, bool rtl, bool completable)
{
	if (schedule_redraw_) {
		// This happens when a graphic conversion is finished. As we don't know
		// the size of the new graphics, it's better the update everything.
		// We can't use redraw() here because this would trigger a infinite
		// recursive loop with showCursor().
		buffer_view_->resize(viewport()->width(), viewport()->height());
		updateScreen();
		updateScrollbar();
		viewport()->update(QRect(0, 0, viewport()->width(), viewport()->height()));
		schedule_redraw_ = false;
		// Show the cursor immediately after the update.
		hideCursor();
		toggleCursor();
		return;
	}

	cursor_->update(x, y, h, l_shape, rtl, completable);
	cursor_->show();
	viewport()->update(cursor_->rect());
}


void GuiWorkArea::removeCursor()
{
	cursor_->hide();
	//if (!qApp->focusWidget())
		viewport()->update(cursor_->rect());
}


void GuiWorkArea::inputMethodEvent(QInputMethodEvent * e)
{
	QString const & commit_string = e->commitString();
	docstring const & preedit_string
		= qstring_to_ucs4(e->preeditString());

	if (!commit_string.isEmpty()) {

		LYXERR(Debug::KEY, "preeditString: " << e->preeditString()
			<< " commitString: " << e->commitString());

		int key = 0;

		// FIXME Iwami 04/01/07: we should take care also of UTF16 surrogates here.
		for (int i = 0; i != commit_string.size(); ++i) {
			QKeyEvent ev(QEvent::KeyPress, key, Qt::NoModifier, commit_string[i]);
			keyPressEvent(&ev);
		}
	}

	// Hide the cursor during the kana-kanji transformation.
	if (preedit_string.empty())
		startBlinkingCursor();
	else
		stopBlinkingCursor();

	// last_width : for checking if last preedit string was/wasn't empty.
	static bool last_width = false;
	if (!last_width && preedit_string.empty()) {
		// if last_width is last length of preedit string.
		e->accept();
		return;
	}

	GuiPainter pain(&screen_);
	buffer_view_->updateMetrics();
	buffer_view_->draw(pain);
	FontInfo font = buffer_view_->cursor().getFont().fontInfo();
	FontMetrics const & fm = theFontMetrics(font);
	int height = fm.maxHeight();
	int cur_x = cursor_->rect().left();
	int cur_y = cursor_->rect().bottom();

	// redraw area of preedit string.
	update(0, cur_y - height, viewport()->width(),
		(height + 1) * preedit_lines_);

	if (preedit_string.empty()) {
		last_width = false;
		preedit_lines_ = 1;
		e->accept();
		return;
	}
	last_width = true;

	// att : stores an IM attribute.
	QList<QInputMethodEvent::Attribute> const & att = e->attributes();

	// get attributes of input method cursor.
	// cursor_pos : cursor position in preedit string.
	size_t cursor_pos = 0;
	bool cursor_is_visible = false;
	for (int i = 0; i != att.size(); ++i) {
		if (att.at(i).type == QInputMethodEvent::Cursor) {
			cursor_pos = att.at(i).start;
			cursor_is_visible = att.at(i).length != 0;
			break;
		}
	}

	size_t preedit_length = preedit_string.length();

	// get position of selection in input method.
	// FIXME: isn't there a way to do this simplier?
	// rStart : cursor position in selected string in IM.
	size_t rStart = 0;
	// rLength : selected string length in IM.
	size_t rLength = 0;
	if (cursor_pos < preedit_length) {
		for (int i = 0; i != att.size(); ++i) {
			if (att.at(i).type == QInputMethodEvent::TextFormat) {
				if (att.at(i).start <= int(cursor_pos)
					&& int(cursor_pos) < att.at(i).start + att.at(i).length) {
						rStart = att.at(i).start;
						rLength = att.at(i).length;
						if (!cursor_is_visible)
							cursor_pos += rLength;
						break;
				}
			}
		}
	}
	else {
		rStart = cursor_pos;
		rLength = 0;
	}

	int const right_margin = buffer_view_->rightMargin();
	Painter::preedit_style ps;
	// Most often there would be only one line:
	preedit_lines_ = 1;
	for (size_t pos = 0; pos != preedit_length; ++pos) {
		char_type const typed_char = preedit_string[pos];
		// reset preedit string style
		ps = Painter::preedit_default;

		// if we reached the right extremity of the screen, go to next line.
		if (cur_x + fm.width(typed_char) > viewport()->width() - right_margin) {
			cur_x = right_margin;
			cur_y += height + 1;
			++preedit_lines_;
		}
		// preedit strings are displayed with dashed underline
		// and partial strings are displayed white on black indicating
		// that we are in selecting mode in the input method.
		// FIXME: rLength == preedit_length is not a changing condition
		// FIXME: should be put out of the loop.
		if (pos >= rStart
			&& pos < rStart + rLength
			&& !(cursor_pos < rLength && rLength == preedit_length))
			ps = Painter::preedit_selecting;

		if (pos == cursor_pos
			&& (cursor_pos < rLength && rLength == preedit_length))
			ps = Painter::preedit_cursor;

		// draw one character and update cur_x.
		cur_x += pain.preeditText(cur_x, cur_y, typed_char, font, ps);
	}

	// update the preedit string screen area.
	update(0, cur_y - preedit_lines_*height, viewport()->width(),
		(height + 1) * preedit_lines_);

	// Don't forget to accept the event!
	e->accept();
}


QVariant GuiWorkArea::inputMethodQuery(Qt::InputMethodQuery query) const
{
	QRect cur_r(0,0,0,0);
	switch (query) {
		// this is the CJK-specific composition window position.
		case Qt::ImMicroFocus:
			cur_r = cursor_->rect();
			if (preedit_lines_ != 1)
				cur_r.moveLeft(10);
			cur_r.moveBottom(cur_r.bottom() + cur_r.height() * preedit_lines_);
			// return lower right of cursor in LyX.
			return cur_r;
		default:
			return QWidget::inputMethodQuery(query);
	}
}


void GuiWorkArea::updateWindowTitle()
{
	docstring maximize_title;
	docstring minimize_title;

	Buffer & buf = buffer_view_->buffer();
	FileName const fileName = buf.fileName();
	if (!fileName.empty()) {
		maximize_title = fileName.displayName(30);
		minimize_title = from_utf8(fileName.onlyFileName());
		if (!buf.isClean()) {
			maximize_title += _(" (changed)");
			minimize_title += char_type('*');
		}
		if (buf.isReadonly())
			maximize_title += _(" (read only)");
	}

	QString title = windowTitle();
	QString new_title = toqstr(maximize_title);
	if (title == new_title)
		return;

	QWidget::setWindowTitle(new_title);
	QWidget::setWindowIconText(toqstr(minimize_title));
	titleChanged(this);
}


void GuiWorkArea::setReadOnly(bool)
{
	updateWindowTitle();
	if (this == lyx_view_->currentWorkArea())
		lyx_view_->updateDialogs();
}


bool GuiWorkArea::isFullScreen()
{
	return lyx_view_ && lyx_view_->isFullScreen();
}


////////////////////////////////////////////////////////////////////
//
// TabWorkArea 
//
////////////////////////////////////////////////////////////////////

#ifdef Q_WS_MACX
class NoTabFrameMacStyle : public QMacStyle {
public:
	///
	QRect subElementRect(SubElement element, const QStyleOption * option,
			     const QWidget * widget = 0) const
	{
		QRect rect = QMacStyle::subElementRect(element, option, widget);
		bool noBar = static_cast<QTabWidget const *>(widget)->count() <= 1;
		
		// The Qt Mac style puts the contents into a 3 pixel wide box
		// which looks very ugly and not like other Mac applications.
		// Hence we remove this here, and moreover the 16 pixel round
		// frame above if the tab bar is hidden.
		if (element == QStyle::SE_TabWidgetTabContents) {
			rect.adjust(- rect.left(), 0, rect.left(), 0);
			if (noBar)
				rect.setTop(0);
		}

		return rect;
	}
};

NoTabFrameMacStyle noTabFrameMacStyle;
#endif


TabWorkArea::TabWorkArea(QWidget * parent)
	: QTabWidget(parent), clicked_tab_(-1)
{
#ifdef Q_WS_MACX
	setStyle(&noTabFrameMacStyle);
#endif

	QPalette pal = palette();
	pal.setColor(QPalette::Active, QPalette::Button,
		pal.color(QPalette::Active, QPalette::Window));
	pal.setColor(QPalette::Disabled, QPalette::Button,
		pal.color(QPalette::Disabled, QPalette::Window));
	pal.setColor(QPalette::Inactive, QPalette::Button,
		pal.color(QPalette::Inactive, QPalette::Window));

	QObject::connect(this, SIGNAL(currentChanged(int)),
		this, SLOT(on_currentTabChanged(int)));

	QToolButton * closeBufferButton = new QToolButton(this);
	closeBufferButton->setPalette(pal);
	// FIXME: rename the icon to closebuffer.png
	closeBufferButton->setIcon(QIcon(":/images/closetab.png"));
	closeBufferButton->setText("Close File");
	closeBufferButton->setAutoRaise(true);
	closeBufferButton->setCursor(Qt::ArrowCursor);
	closeBufferButton->setToolTip(qt_("Close File"));
	closeBufferButton->setEnabled(true);
	QObject::connect(closeBufferButton, SIGNAL(clicked()),
		this, SLOT(closeCurrentBuffer()));
	setCornerWidget(closeBufferButton, Qt::TopRightCorner);
	
	// setup drag'n'drop
	QTabBar* tb = new DragTabBar;
	connect(tb, SIGNAL(tabMoveRequested(int, int)),
		this, SLOT(moveTab(int, int)));
	tb->setElideMode(Qt::ElideNone);
	setTabBar(tb);

	// make us responsible for the context menu of the tabbar
	tb->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tb, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(showContextMenu(const QPoint &)));
	
	setUsesScrollButtons(true);
}


void TabWorkArea::setFullScreen(bool full_screen)
{
	for (int i = 0; i != count(); ++i) {
		if (GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(widget(i)))
			wa->setFullScreen(full_screen);
	}

	if (lyxrc.full_screen_tabbar)
		showBar(!full_screen && count()>1);
}


void TabWorkArea::showBar(bool show)
{
	tabBar()->setEnabled(show);
	tabBar()->setVisible(show);
}


GuiWorkArea * TabWorkArea::currentWorkArea()
{
	if (count() == 0)
		return 0;

	GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(currentWidget()); 
	LASSERT(wa, /**/);
	return wa;
}


GuiWorkArea * TabWorkArea::workArea(Buffer & buffer)
{
	for (int i = 0; i != count(); ++i) {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(widget(i));
		LASSERT(wa, return 0);
		if (&wa->bufferView().buffer() == &buffer)
			return wa;
	}
	return 0;
}


void TabWorkArea::closeAll()
{
	while (count()) {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(widget(0));
		LASSERT(wa, /**/);
		removeTab(0);
		delete wa;
	}
}


bool TabWorkArea::setCurrentWorkArea(GuiWorkArea * work_area)
{
	LASSERT(work_area, /**/);
	int index = indexOf(work_area);
	if (index == -1)
		return false;

	if (index == currentIndex())
		// Make sure the work area is up to date.
		on_currentTabChanged(index);
	else
		// Switch to the work area.
		setCurrentIndex(index);
	work_area->setFocus();

	return true;
}


GuiWorkArea * TabWorkArea::addWorkArea(Buffer & buffer, GuiView & view)
{
	GuiWorkArea * wa = new GuiWorkArea(buffer, view);
	wa->setUpdatesEnabled(false);
	// Hide tabbar if there's no tab (avoid a resize and a flashing tabbar
	// when hiding it again below).
	if (!(currentWorkArea() && currentWorkArea()->isFullScreen()))
		showBar(count() > 0);
	addTab(wa, wa->windowTitle());
	QObject::connect(wa, SIGNAL(titleChanged(GuiWorkArea *)),
		this, SLOT(updateTabTexts()));
	if (currentWorkArea() && currentWorkArea()->isFullScreen())
		setFullScreen(true);
	else
		// Hide tabbar if there's only one tab.
		showBar(count() > 1);

	updateTabTexts();
	
	return wa;
}


bool TabWorkArea::removeWorkArea(GuiWorkArea * work_area)
{
	LASSERT(work_area, return false);
	int index = indexOf(work_area);
	if (index == -1)
		return false;

	work_area->setUpdatesEnabled(false);
	removeTab(index);
	delete work_area;

	if (count()) {
		// make sure the next work area is enabled.
		currentWidget()->setUpdatesEnabled(true);
		if (currentWorkArea() && currentWorkArea()->isFullScreen())
			setFullScreen(true);
		else
			// Hide tabbar if there's only one tab.
			showBar(count() > 1);
	} else {
		lastWorkAreaRemoved();
	}

	updateTabTexts();

	return true;
}


void TabWorkArea::on_currentTabChanged(int i)
{
	// returns e.g. on application destruction
	if (i == -1)
		return;
	GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(widget(i));
	LASSERT(wa, return);
	BufferView & bv = wa->bufferView();
	bv.cursor().fixIfBroken();
	bv.updateMetrics();
	wa->setUpdatesEnabled(true);
	wa->redraw();
	wa->setFocus();
	///
	currentWorkAreaChanged(wa);

	LYXERR(Debug::GUI, "currentTabChanged " << i
		<< "File" << bv.buffer().absFileName());
}


void TabWorkArea::closeCurrentBuffer()
{
	if (clicked_tab_ != -1)
		setCurrentIndex(clicked_tab_);

	lyx::dispatch(FuncRequest(LFUN_BUFFER_CLOSE));
}


void TabWorkArea::closeCurrentTab()
{
	if (clicked_tab_ == -1)
		removeWorkArea(currentWorkArea());
	else {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(widget(clicked_tab_)); 
		LASSERT(wa, /**/);
		removeWorkArea(wa);
	}
}

///
class DisplayPath {
public:
	/// make vector happy
	DisplayPath() {}
	///
	DisplayPath(int tab, FileName const & filename)
		: tab_(tab)
	{
		filename_ = toqstr(filename.onlyFileNameWithoutExt());
		postfix_ = toqstr(filename.absoluteFilePath()).
			split("/", QString::SkipEmptyParts);
		postfix_.pop_back();
		abs_ = toqstr(filename.absoluteFilePath());
		dottedPrefix_ = false;
	}
	
	/// Absolute path for debugging.
	QString abs() const
	{
		return abs_;
	}
	/// Add the first segment from the postfix or three dots to the prefix.
	/// Merge multiple dot tripples. In fact dots are added lazily, i.e. only
	/// when really needed.
	void shiftPathSegment(bool dotted)
	{
		if (postfix_.count() <= 0)
			return;

		if (!dotted) {
			if (dottedPrefix_ && !prefix_.isEmpty())
				prefix_ += ".../";
			prefix_ += postfix_.front() + "/";
		}
		dottedPrefix_ = dotted && !prefix_.isEmpty();
		postfix_.pop_front();
	}
	///
	QString displayString() const
	{
		if (prefix_.isEmpty())
			return filename_;

		bool dots = dottedPrefix_ || !postfix_.isEmpty();
		return prefix_ + (dots ? ".../" : "") + filename_;
	}
	///
	QString forecastPathString() const
	{
		if (postfix_.count() == 0)
			return displayString();
		
		return prefix_
			+ (dottedPrefix_ ? ".../" : "")
			+ postfix_.front() + "/";
	}
	///
	bool final() const { return postfix_.empty(); }
	///
	int tab() const { return tab_; }
	
private:
	///
	QString prefix_;
	///
	QStringList postfix_;
	///
	QString filename_;
	///
	QString abs_;
	///
	int tab_;
	///
	bool dottedPrefix_;
};


///
bool operator<(DisplayPath const & a, DisplayPath const & b)
{
	return a.displayString() < b.displayString();
}

///
bool operator==(DisplayPath const & a, DisplayPath const & b)
{
	return a.displayString() == b.displayString();
}


void TabWorkArea::updateTabTexts()
{
	size_t n = count();
	if (n == 0)
		return;
	std::list<DisplayPath> paths;
	typedef std::list<DisplayPath>::iterator It;
	
	// collect full names first: path into postfix, empty prefix and 
	// filename without extension
	for (size_t i = 0; i < n; ++i) {
		GuiWorkArea * i_wa = dynamic_cast<GuiWorkArea *>(widget(i)); 
		FileName const fn = i_wa->bufferView().buffer().fileName();
		paths.push_back(DisplayPath(i, fn));
	}
	
	// go through path segments and see if it helps to make the path more unique
	bool somethingChanged = true;
	bool allFinal = false;
	while (somethingChanged && !allFinal) {
		// adding path segments changes order
		paths.sort();
		
		LYXERR(Debug::GUI, "updateTabTexts() iteration start");
		somethingChanged = false;
		allFinal = true;
		
		// find segments which are not unique (i.e. non-atomic)
		It it = paths.begin();
		It segStart = it;
		QString segString = it->displayString();
		for (; it != paths.end(); ++it) {
			// look to the next item
			It next = it;
			++next;
			
			// final?
			allFinal = allFinal && it->final();
			
			LYXERR(Debug::GUI, "it = " << it->abs()
			       << " => " << it->displayString());
			
			// still the same segment?
			QString nextString;
			if ((next != paths.end()
			     && (nextString = next->displayString()) == segString))
				continue;
			LYXERR(Debug::GUI, "segment ended");
			
			// only a trivial one with one element?
			if (it == segStart) {
				// start new segment
				segStart = next;
				segString = nextString;
				continue;
			}
			
			// we found a non-atomic segment segStart <= sit <= it < next.
			// Shift path segments and hope for the best
			// that it makes the path more unique.
			somethingChanged = true;
			It sit = segStart;
			QString dspString = sit->forecastPathString();
			LYXERR(Debug::GUI, "first forecast found for "
			       << sit->abs() << " => " << dspString);
			++sit;
			bool moreUnique = false;
			for (; sit != next; ++sit) {
				if (sit->forecastPathString() != dspString) {
					LYXERR(Debug::GUI, "different forecast found for "
						<< sit->abs() << " => " << sit->forecastPathString());
					moreUnique = true;
					break;
				}
				LYXERR(Debug::GUI, "same forecast found for "
					<< sit->abs() << " => " << dspString);
			}
			
			// if the path segment helped, add it. Otherwise add dots
			bool dots = !moreUnique;
			LYXERR(Debug::GUI, "using dots = " << dots);
			for (sit = segStart; sit != next; ++sit) {
				sit->shiftPathSegment(dots);
				LYXERR(Debug::GUI, "shifting "
					<< sit->abs() << " => " << sit->displayString());
			}

			// start new segment
			segStart = next;
			segString = nextString;
		}
	}
	
	// set new tab titles
	for (It it = paths.begin(); it != paths.end(); ++it) {
		GuiWorkArea * i_wa = dynamic_cast<GuiWorkArea *>(widget(it->tab())); 
		Buffer & buf = i_wa->bufferView().buffer();
		if (!buf.fileName().empty() && !buf.isClean())
			setTabText(it->tab(), it->displayString() + "*");
		else
			setTabText(it->tab(), it->displayString());
	}
}


void TabWorkArea::showContextMenu(const QPoint & pos)
{
	// which tab?
	clicked_tab_ = static_cast<DragTabBar *>(tabBar())->tabAt(pos);
	if (clicked_tab_ == -1)
		return;
	
	// show tab popup
	QMenu popup;
	popup.addAction(QIcon(":/images/hidetab.png"),
		qt_("Hide tab"), this, SLOT(closeCurrentTab()));
	popup.addAction(QIcon(":/images/closetab.png"),
		qt_("Close tab"), this, SLOT(closeCurrentBuffer()));
	popup.exec(tabBar()->mapToGlobal(pos));

	clicked_tab_ = -1;
}


void TabWorkArea::moveTab(int fromIndex, int toIndex)
{
	QWidget * w = widget(fromIndex);
	QIcon icon = tabIcon(fromIndex);
	QString text = tabText(fromIndex);

	setCurrentIndex(fromIndex);
	removeTab(fromIndex);
	insertTab(toIndex, w, icon, text);
	setCurrentIndex(toIndex);
}
	

DragTabBar::DragTabBar(QWidget* parent)
	: QTabBar(parent)
{
	setAcceptDrops(true);
}


#if QT_VERSION < 0x040300
int DragTabBar::tabAt(QPoint const & position) const
{
	const int max = count();
	for (int i = 0; i < max; ++i) {
		if (tabRect(i).contains(position))
			return i;
	}
	return -1;
}
#endif


void DragTabBar::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
		dragStartPos_ = event->pos();
	QTabBar::mousePressEvent(event);
}


void DragTabBar::mouseMoveEvent(QMouseEvent * event)
{
	// If the left button isn't pressed anymore then return
	if (!(event->buttons() & Qt::LeftButton))
		return;
	
	// If the distance is too small then return
	if ((event->pos() - dragStartPos_).manhattanLength()
	    < QApplication::startDragDistance())
		return;

	// did we hit something after all?
	int tab = tabAt(dragStartPos_);
	if (tab == -1)
		return;
	
	// simulate button release to remove highlight from button
	int i = currentIndex();
	QMouseEvent me(QEvent::MouseButtonRelease, dragStartPos_,
		event->button(), event->buttons(), 0);
	QTabBar::mouseReleaseEvent(&me);
	setCurrentIndex(i);
	
	// initiate Drag
	QDrag * drag = new QDrag(this);
	QMimeData * mimeData = new QMimeData;
	// a crude way to distinguish tab-reodering drops from other ones
	mimeData->setData("action", "tab-reordering") ;
	drag->setMimeData(mimeData);
	
#if QT_VERSION >= 0x040300
	// get tab pixmap as cursor
	QRect r = tabRect(tab);
	QPixmap pixmap(r.size());
	render(&pixmap, - r.topLeft());
	drag->setPixmap(pixmap);
	drag->exec();
#else
	drag->start(Qt::MoveAction);
#endif
	
}


void DragTabBar::dragEnterEvent(QDragEnterEvent * event)
{
	// Only accept if it's an tab-reordering request
	QMimeData const * m = event->mimeData();
	QStringList formats = m->formats();
	if (formats.contains("action") 
	    && m->data("action") == "tab-reordering")
		event->acceptProposedAction();
}


void DragTabBar::dropEvent(QDropEvent * event)
{
	int fromIndex = tabAt(dragStartPos_);
	int toIndex = tabAt(event->pos());
	
	// Tell interested objects that 
	if (fromIndex != toIndex)
		tabMoveRequested(fromIndex, toIndex);
	event->acceptProposedAction();
}


} // namespace frontend
} // namespace lyx

#include "GuiWorkArea_moc.cpp"
