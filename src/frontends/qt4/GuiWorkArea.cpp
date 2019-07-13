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
#include "GuiWorkArea_Private.h"

#include "ColorCache.h"
#include "FontLoader.h"
#include "GuiApplication.h"
#include "GuiCompleter.h"
#include "GuiKeySymbol.h"
#include "GuiPainter.h"
#include "GuiView.h"
#include "Menus.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "Font.h"
#include "FuncRequest.h"
#include "KeySymbol.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Text.h"
#include "TextMetrics.h"
#include "Undo.h"
#include "version.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"
#include "support/TempFile.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/WorkAreaManager.h"

#include <QContextMenuEvent>
#if (QT_VERSION < 0x050000)
#include <QInputContext>
#endif
#include <QDrag>
#include <QHelpEvent>
#ifdef Q_OS_MAC
#include <QProxyStyle>
#endif
#include <QMainWindow>
#include <QMimeData>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QStyleOption>
#include <QStylePainter>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QMenuBar>

#include <cmath>
#include <iostream>

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

class CaretWidget {
public:
	CaretWidget() : rtl_(false), l_shape_(false), completable_(false),
		x_(0), caret_width_(0)
	{}

	/* Draw the caret. Parameter \c horiz_offset is not 0 when there
	 * has been horizontal scrolling in current row
	 */
	void draw(QPainter & painter, int horiz_offset)
	{
		if (!rect_.isValid())
			return;

		int const x = x_ - horiz_offset;
		int const y = rect_.top();
		int const l = x_ - rect_.left();
		int const r = rect_.right() - x_;
		int const bot = rect_.bottom();

		// draw vertical line
		painter.fillRect(x, y, caret_width_, rect_.height(), color_);

		// draw RTL/LTR indication
		painter.setPen(color_);
		if (l_shape_) {
			if (rtl_)
				painter.drawLine(x, bot, x - l, bot);
			else
				painter.drawLine(x, bot, x + caret_width_ + r, bot);
		}

		// draw completion triangle
		if (completable_) {
			int m = y + rect_.height() / 2;
			int d = TabIndicatorWidth - 1;
			if (rtl_) {
				painter.drawLine(x - 1, m - d, x - 1 - d, m);
				painter.drawLine(x - 1, m + d, x - 1 - d, m);
			} else {
				painter.drawLine(x + caret_width_, m - d, x + caret_width_ + d, m);
				painter.drawLine(x + caret_width_, m + d, x + caret_width_ + d, m);
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

		//FIXME: LyXRC::cursor_width should be caret_width
		caret_width_ = lyxrc.cursor_width
			? lyxrc.cursor_width
			: 1 + int((lyxrc.currentZoom + 50) / 200.0);

		// compute overall rectangle
		rect_ = QRect(x - l, y, caret_width_ + r + l, h);
	}

	QRect const & rect() { return rect_; }

private:
	/// caret is in RTL or LTR text
	bool rtl_;
	/// indication for RTL or LTR
	bool l_shape_;
	/// triangle to show that a completion is available
	bool completable_;
	///
	QColor color_;
	/// rectangle, possibly with l_shape and completion triangle
	QRect rect_;
	/// x position (were the vertical line is drawn)
	int x_;
	/// the width of the vertical blinking bar
	int caret_width_;
};


// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true)
{}


GuiWorkArea::Private::Private(GuiWorkArea * parent)
: p(parent), buffer_view_(0), lyx_view_(0), caret_(0),
  caret_visible_(false), need_resize_(false), preedit_lines_(1),
  last_pixel_ratio_(1.0), completer_(new GuiCompleter(p, p)),
  dialog_mode_(false), shell_escape_(false), read_only_(false),
  clean_(true), externally_modified_(false)
{
/* Qt on macOS and Wayland does not respect the
 * Qt::WA_OpaquePaintEvent attribute and resets the widget backing
 * store at each update. Therefore, we use our own backing store in
 * these two cases. */
#if QT_VERSION >= 0x050000
	use_backingstore_ = guiApp->platformName() == "cocoa"
		|| guiApp->platformName().contains("wayland");
#else
#  ifdef Q_OS_MAC
	use_backingstore_ = true;
#  else
	use_backingstore_ = false;
#  endif
#endif

	int const time = QApplication::cursorFlashTime() / 2;
	if (time > 0) {
		caret_timeout_.setInterval(time);
		caret_timeout_.start();
	} else {
		// let's initialize this just to be safe
		caret_timeout_.setInterval(500);
	}
}


GuiWorkArea::Private::~Private()
{
	// If something is wrong with the buffer, we can ignore it safely
	try {
		buffer_view_->buffer().workAreaManager().remove(p);
	} catch(...) {}
	delete buffer_view_;
	delete caret_;
	// Completer has a QObject parent and is thus automatically destroyed.
	// See #4758.
	// delete completer_;
}


GuiWorkArea::GuiWorkArea(QWidget * /* w */)
: d(new Private(this))
{
	new CompressorProxy(this); // not a leak
}


GuiWorkArea::GuiWorkArea(Buffer & buffer, GuiView & gv)
: d(new Private(this))
{
	new CompressorProxy(this); // not a leak
	setGuiView(gv);
	buffer.params().display_pixel_ratio = theGuiApp()->pixelRatio();
	setBuffer(buffer);
	init();
}


double GuiWorkArea::pixelRatio() const
{
#if QT_VERSION >= 0x050000
	return qt_scale_factor * devicePixelRatio();
#else
	return 1.0;
#endif
}


void GuiWorkArea::init()
{
	// Setup the signals
	connect(&d->caret_timeout_, SIGNAL(timeout()),
		this, SLOT(toggleCaret()));

	// This connection is closed at the same time as this is destroyed.
	d->synthetic_mouse_event_.timeout.timeout.connect([this](){
			generateSyntheticMouseEvent();
		});

	d->resetScreen();
	// With Qt4.5 a mouse event will happen before the first paint event
	// so make sure that the buffer view has an up to date metrics.
	d->buffer_view_->resize(viewport()->width(), viewport()->height());
	d->caret_ = new frontend::CaretWidget();

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptDrops(true);
	setMouseTracking(true);
	setMinimumSize(100, 70);
	setFrameStyle(QFrame::NoFrame);
	updateWindowTitle();

	d->updateCursorShape();

	// we paint our own background
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);

	setFocusPolicy(Qt::StrongFocus);

	LYXERR(Debug::GUI, "viewport width: " << viewport()->width()
		<< "  viewport height: " << viewport()->height());

	// Enables input methods for asian languages.
	// Must be set when creating custom text editing widgets.
	setAttribute(Qt::WA_InputMethodEnabled, true);
}


GuiWorkArea::~GuiWorkArea()
{
	delete d;
}


void GuiWorkArea::Private::updateCursorShape()
{
	bool const clickable = buffer_view_ && buffer_view_->clickableInset();
	p->viewport()->setCursor(clickable ? Qt::PointingHandCursor
	                                   : Qt::IBeamCursor);
}


void GuiWorkArea::setGuiView(GuiView & gv)
{
	d->lyx_view_ = &gv;
}


void GuiWorkArea::setBuffer(Buffer & buffer)
{
	delete d->buffer_view_;
	d->buffer_view_ = new BufferView(buffer);
	buffer.workAreaManager().add(this);

	// HACK: Prevents an additional redraw when the scrollbar pops up
	// which regularily happens on documents with more than one page.
	// The policy  should be set to "Qt::ScrollBarAsNeeded" soon.
	// Since we have no geometry information yet, we assume that
	// a document needs a scrollbar if there is more then four
	// paragraph in the outermost text.
	if (buffer.text().paragraphs().size() > 4)
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	QTimer::singleShot(50, this, SLOT(fixVerticalScrollBar()));
	Q_EMIT bufferViewChanged();
}


void GuiWorkArea::fixVerticalScrollBar()
{
	if (!isFullScreen())
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


void GuiWorkArea::close()
{
	d->lyx_view_->removeWorkArea(this);
}


void GuiWorkArea::setFullScreen(bool full_screen)
{
	d->buffer_view_->setFullScreen(full_screen);
	setFrameStyle(QFrame::NoFrame);
	if (full_screen) {
		setFrameStyle(QFrame::NoFrame);
		if (lyxrc.full_screen_scrollbar)
			setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


BufferView & GuiWorkArea::bufferView()
{
	return *d->buffer_view_;
}


BufferView const & GuiWorkArea::bufferView() const
{
	return *d->buffer_view_;
}


void GuiWorkArea::stopBlinkingCaret()
{
	d->caret_timeout_.stop();
	d->hideCaret();
}


void GuiWorkArea::startBlinkingCaret()
{
	// do not show the cursor if the view is busy
	if (view().busy())
		return;

	Point p;
	int h = 0;
	d->buffer_view_->caretPosAndHeight(p, h);
	// Don't start blinking if the cursor isn't on screen.
	if (!d->buffer_view_->cursorInView(p, h))
		return;

	d->showCaret();

	//we're not supposed to cache this value.
	int const time = QApplication::cursorFlashTime() / 2;
	if (time <= 0)
		return;
	d->caret_timeout_.setInterval(time);
	d->caret_timeout_.start();
}


void GuiWorkArea::toggleCaret()
{
	if (d->caret_visible_)
		d->hideCaret();
	else
		d->showCaret();
}


void GuiWorkArea::scheduleRedraw(bool update_metrics)
{
	if (!isVisible())
		// No need to redraw in this case.
		return;

	// No need to do anything if this is the current view. The BufferView
	// metrics are already up to date.
	if (update_metrics || d->lyx_view_ != guiApp->currentView()
		|| d->lyx_view_->currentWorkArea() != this) {
		// FIXME: it would be nice to optimize for the off-screen case.
		d->buffer_view_->cursor().fixIfBroken();
		d->buffer_view_->updateMetrics();
		d->buffer_view_->cursor().fixIfBroken();
	}

	// update caret position, because otherwise it has to wait until
	// the blinking interval is over
	d->updateCaretGeometry();

	LYXERR(Debug::WORKAREA, "WorkArea::redraw screen");
	viewport()->update();

	/// FIXME: is this still true now that paintEvent does the actual painting?
	/// \warning: scrollbar updating *must* be done after the BufferView is drawn
	/// because \c BufferView::updateScrollbar() is called in \c BufferView::draw().
	d->updateScrollbar();
	d->lyx_view_->updateStatusBar();

	if (lyxerr.debugging(Debug::WORKAREA))
		d->buffer_view_->coordCache().dump();

	updateWindowTitle();

	d->updateCursorShape();
}


// Keep in sync with GuiWorkArea::processKeySym below
bool GuiWorkArea::queryKeySym(KeySymbol const & key, KeyModifier mod) const
{
	return guiApp->queryKeySym(key, mod);
}


// Keep in sync with GuiWorkArea::queryKeySym above
void GuiWorkArea::processKeySym(KeySymbol const & key, KeyModifier mod)
{
	if (d->lyx_view_->isFullScreen() && d->lyx_view_->menuBar()->isVisible()
		&& lyxrc.full_screen_menubar) {
		// FIXME HACK: we should not have to do this here. See related comment
		// in GuiView::event() (QEvent::ShortcutOverride)
		d->lyx_view_->menuBar()->hide();
	}

	// In order to avoid bad surprise in the middle of an operation,
	// we better stop the blinking caret...
	// the caret gets restarted in GuiView::restartCaret()
	stopBlinkingCaret();
	guiApp->processKeySym(key, mod);
}


void GuiWorkArea::Private::dispatch(FuncRequest const & cmd)
{
	// Handle drag&drop
	if (cmd.action() == LFUN_FILE_OPEN) {
		DispatchResult dr;
		lyx_view_->dispatch(cmd, dr);
		return;
	}

	bool const notJustMovingTheMouse =
		cmd.action() != LFUN_MOUSE_MOTION || cmd.button() != mouse_button::none;

	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking caret.
	if (notJustMovingTheMouse)
		p->stopBlinkingCaret();

	buffer_view_->mouseEventDispatch(cmd);

	// Skip these when selecting
	// FIXME: let GuiView take care of those.
	if (cmd.action() != LFUN_MOUSE_MOTION) {
		completer_->updateVisibility(false, false);
		lyx_view_->updateDialogs();
		lyx_view_->updateStatusBar();
	}

	// GUI tweaks except with mouse motion with no button pressed.
	if (notJustMovingTheMouse) {
		// Slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		// FIXME: let GuiView take care of those.
		lyx_view_->clearMessage();

		// Show the caret immediately after any operation
		p->startBlinkingCaret();
	}

	updateCursorShape();
}


void GuiWorkArea::Private::resizeBufferView()
{
	// WARNING: Please don't put any code that will trigger a repaint here!
	// We are already inside a paint event.
	p->stopBlinkingCaret();
	// Warn our container (GuiView).
	p->busy(true);

	Point point;
	int h = 0;
	buffer_view_->caretPosAndHeight(point, h);
	bool const caret_in_view = buffer_view_->cursorInView(point, h);
	buffer_view_->resize(p->viewport()->width(), p->viewport()->height());
	if (caret_in_view)
		buffer_view_->scrollToCursor();
	updateCaretGeometry();

	// Update scrollbars which might have changed due different
	// BufferView dimension. This is especially important when the
	// BufferView goes from zero-size to the real-size for the first time,
	// as the scrollbar paramters are then set for the first time.
	updateScrollbar();

	need_resize_ = false;
	p->busy(false);
	// Eventually, restart the caret after the resize event.
	// We might be resizing even if the focus is on another widget so we only
	// restart the caret if we have the focus.
	if (p->hasFocus())
		QTimer::singleShot(50, p, SLOT(startBlinkingCaret()));
}


void GuiWorkArea::Private::updateCaretGeometry()
{
	Point point;
	int h = 0;
	buffer_view_->caretPosAndHeight(point, h);
	if (!buffer_view_->cursorInView(point, h))
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

	// show caret on screen
	Cursor & cur = buffer_view_->cursor();
	bool completable = cur.inset().showCompletionCursor()
		&& completer_->completionAvailable()
		&& !completer_->popupVisible()
		&& !completer_->inlineVisible();
	caret_visible_ = true;

	caret_->update(point.x_, point.y_, h, l_shape, isrtl, completable);
}


void GuiWorkArea::Private::showCaret()
{
	if (caret_visible_)
		return;

	updateCaretGeometry();
	p->viewport()->update();
}


void GuiWorkArea::Private::hideCaret()
{
	if (!caret_visible_)
		return;

	caret_visible_ = false;
	//if (!qApp->focusWidget())
		p->viewport()->update();
}


void GuiWorkArea::Private::updateScrollbar()
{
	// Prevent setRange() and setSliderPosition from causing recursive calls via
	// the signal valueChanged. (#10311)
	QObject::disconnect(p->verticalScrollBar(), SIGNAL(valueChanged(int)),
	                    p, SLOT(scrollTo(int)));
	ScrollbarParameters const & scroll_ = buffer_view_->scrollbarParameters();
	p->verticalScrollBar()->setRange(scroll_.min, scroll_.max);
	p->verticalScrollBar()->setPageStep(scroll_.page_step);
	p->verticalScrollBar()->setSingleStep(scroll_.single_step);
	p->verticalScrollBar()->setSliderPosition(0);
	// Connect to the vertical scroll bar
	QObject::connect(p->verticalScrollBar(), SIGNAL(valueChanged(int)),
	                 p, SLOT(scrollTo(int)));
}


void GuiWorkArea::scrollTo(int value)
{
	stopBlinkingCaret();
	d->buffer_view_->scrollDocView(value, true);

	if (lyxrc.cursor_follows_scrollbar) {
		d->buffer_view_->setCursorFromScrollbar();
		// FIXME: let GuiView take care of those.
		d->lyx_view_->updateLayoutList();
	}
	// Show the caret immediately after any operation.
	startBlinkingCaret();
	// FIXME QT5
#ifdef Q_WS_X11
	QApplication::syncX();
#endif
}


bool GuiWorkArea::event(QEvent * e)
{
	switch (e->type()) {
	case QEvent::ToolTip: {
		QHelpEvent * helpEvent = static_cast<QHelpEvent *>(e);
		if (lyxrc.use_tooltip) {
			QPoint pos = helpEvent->pos();
			if (pos.x() < viewport()->width()) {
				QString s = toqstr(d->buffer_view_->toolTip(pos.x(), pos.y()));
				QToolTip::showText(helpEvent->globalPos(), formatToolTip(s,35));
			}
			else
				QToolTip::hideText();
		}
		// Don't forget to accept the event!
		e->accept();
		return true;
	}

	case QEvent::ShortcutOverride:
		// keyPressEvent is ShortcutOverride-aware and only accepts the event in
		// this case
		keyPressEvent(static_cast<QKeyEvent *>(e));
		return e->isAccepted();

	case QEvent::KeyPress: {
		// We catch this event in order to catch the Tab or Shift+Tab key press
		// which are otherwise reserved to focus switching between controls
		// within a dialog.
		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		if ((ke->key() == Qt::Key_Tab && ke->modifiers() == Qt::NoModifier)
			|| (ke->key() == Qt::Key_Backtab && (
				ke->modifiers() == Qt::ShiftModifier
				|| ke->modifiers() == Qt::NoModifier))) {
			keyPressEvent(ke);
			return true;
		}
		return QAbstractScrollArea::event(e);
	}

	default:
		return QAbstractScrollArea::event(e);
	}
	return false;
}


void GuiWorkArea::contextMenuEvent(QContextMenuEvent * e)
{
	string name;
	if (e->reason() == QContextMenuEvent::Mouse)
		// the menu name is set on mouse press
		name = d->context_menu_name_;
	else {
		QPoint pos = e->pos();
		Cursor const & cur = d->buffer_view_->cursor();
		if (e->reason() == QContextMenuEvent::Keyboard && cur.inTexted()) {
			// Do not access the context menu of math right in front of before
			// the cursor. This does not work when the cursor is in text.
			Inset * inset = cur.paragraph().getInset(cur.pos());
			if (inset && inset->asInsetMath())
				--pos.rx();
			else if (cur.pos() > 0) {
				Inset * inset = cur.paragraph().getInset(cur.pos() - 1);
				if (inset)
					++pos.rx();
			}
		}
		name = d->buffer_view_->contextMenu(pos.x(), pos.y());
	}

	if (name.empty()) {
		e->accept();
		return;
	}
	// always show mnemonics when the keyboard is used to show the context menu
	// FIXME: This should be fixed in Qt itself
	bool const keyboard = (e->reason() == QContextMenuEvent::Keyboard);
	QMenu * menu = guiApp->menus().menu(toqstr(name), *d->lyx_view_, keyboard);
	if (!menu) {
		e->accept();
		return;
	}
	// Position the menu to the right.
	// FIXME: menu position should be different for RTL text.
	menu->exec(e->globalPos());
	e->accept();
}


void GuiWorkArea::focusInEvent(QFocusEvent * e)
{
	LYXERR(Debug::DEBUG, "GuiWorkArea::focusInEvent(): " << this << endl);
	if (d->lyx_view_->currentWorkArea() != this) {
		d->lyx_view_->setCurrentWorkArea(this);
		d->lyx_view_->currentWorkArea()->bufferView().buffer().updateBuffer();
	}

	startBlinkingCaret();
	QAbstractScrollArea::focusInEvent(e);
}


void GuiWorkArea::focusOutEvent(QFocusEvent * e)
{
	LYXERR(Debug::DEBUG, "GuiWorkArea::focusOutEvent(): " << this << endl);
	stopBlinkingCaret();
	QAbstractScrollArea::focusOutEvent(e);
}


void GuiWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (d->dc_event_.active && d->dc_event_ == *e) {
		d->dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE, e->x(), e->y(),
			q_button_state(e->button()), q_key_state(e->modifiers()));
		d->dispatch(cmd);
		e->accept();
		return;
	}

#if (QT_VERSION < 0x050000) && !defined(__HAIKU__)
	inputContext()->reset();
#endif

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
			q_button_state(e->button()), q_key_state(e->modifiers()));
	d->dispatch(cmd);

	// Save the context menu on mouse press, because also the mouse
	// cursor is set on mouse press. Afterwards, we can either release
	// the mousebutton somewhere else, or the cursor might have moved
	// due to the DEPM. We need to do this after the mouse has been
	// set in dispatch(), because the selection state might change.
	if (e->button() == Qt::RightButton)
		d->context_menu_name_ = d->buffer_view_->contextMenu(e->x(), e->y());

	e->accept();
}


void GuiWorkArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (d->synthetic_mouse_event_.timeout.running())
		d->synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			q_button_state(e->button()), q_key_state(e->modifiers()));
	d->dispatch(cmd);
	e->accept();
}


void GuiWorkArea::mouseMoveEvent(QMouseEvent * e)
{
	// we kill the triple click if we move
	doubleClickTimeout();
	FuncRequest cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
			q_motion_state(e->buttons()), q_key_state(e->modifiers()));

	e->accept();

	// If we're above or below the work area...
	if ((e->y() <= 20 || e->y() >= viewport()->height() - 20)
			&& e->buttons() == mouse_button::button1) {
		// Make sure only a synthetic event can cause a page scroll,
		// so they come at a steady rate:
		if (e->y() <= 20)
			// _Force_ a scroll up:
			cmd.set_y(e->y() - 21);
		else
			cmd.set_y(e->y() + 21);
		// Store the event, to be handled when the timeout expires.
		d->synthetic_mouse_event_.cmd = cmd;

		if (d->synthetic_mouse_event_.timeout.running()) {
			// Discard the event. Note that it _may_ be handled
			// when the timeout expires if
			// synthetic_mouse_event_.cmd has not been overwritten.
			// Ie, when the timeout expires, we handle the
			// most recent event but discard all others that
			// occurred after the one used to start the timeout
			// in the first place.
			return;
		}

		d->synthetic_mouse_event_.restart_timeout = true;
		d->synthetic_mouse_event_.timeout.start();
		// Fall through to handle this event...

	} else if (d->synthetic_mouse_event_.timeout.running()) {
		// Store the event, to be possibly handled when the timeout
		// expires.
		// Once the timeout has expired, normal control is returned
		// to mouseMoveEvent (restart_timeout = false).
		// This results in a much smoother 'feel' when moving the
		// mouse back into the work area.
		d->synthetic_mouse_event_.cmd = cmd;
		d->synthetic_mouse_event_.restart_timeout = false;
		return;
	}
	d->dispatch(cmd);
}


void GuiWorkArea::wheelEvent(QWheelEvent * ev)
{
	// Wheel rotation by one notch results in a delta() of 120 (see
	// documentation of QWheelEvent)
	// But first we have to ignore horizontal scroll events.
#if QT_VERSION < 0x050000
	if (ev->orientation() == Qt::Horizontal) {
		ev->accept();
		return;
	}
	double const delta = ev->delta() / 120.0;
#else
	QPoint const aDelta = ev->angleDelta();
	// skip horizontal wheel event
	if (abs(aDelta.x()) > abs(aDelta.y())) {
		ev->accept();
		return;
	}
	double const delta = aDelta.y() / 120.0;
#endif

	bool zoom = false;
	switch (lyxrc.scroll_wheel_zoom) {
	case LyXRC::SCROLL_WHEEL_ZOOM_CTRL:
		zoom = ev->modifiers() & Qt::ControlModifier;
		zoom &= !(ev->modifiers() & (Qt::ShiftModifier | Qt::AltModifier));
		break;
	case LyXRC::SCROLL_WHEEL_ZOOM_SHIFT:
		zoom = ev->modifiers() & Qt::ShiftModifier;
		zoom &= !(ev->modifiers() & (Qt::ControlModifier | Qt::AltModifier));
		break;
	case LyXRC::SCROLL_WHEEL_ZOOM_ALT:
		zoom = ev->modifiers() & Qt::AltModifier;
		zoom &= !(ev->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier));
		break;
	case LyXRC::SCROLL_WHEEL_ZOOM_OFF:
		break;
	}
	if (zoom) {
		docstring arg = convert<docstring>(int(5 * delta));
		lyx::dispatch(FuncRequest(LFUN_BUFFER_ZOOM_IN, arg));
		return;
	}

	// Take into account the desktop wide settings.
	int const lines = qApp->wheelScrollLines();
	int const page_step = verticalScrollBar()->pageStep();
	// Test if the wheel mouse is set to one screen at a time.
	// This is according to
	// https://doc.qt.io/qt-5/qapplication.html#wheelScrollLines-prop
	int scroll_value =
		min(lines * verticalScrollBar()->singleStep(), page_step);

	// Take into account the rotation and the user preferences.
	scroll_value = int(scroll_value * delta * lyxrc.mouse_wheel_speed);
	LYXERR(Debug::SCROLLING, "wheelScrollLines = " << lines
			<< " delta = " << delta << " scroll_value = " << scroll_value
			<< " page_step = " << page_step);
	// Now scroll.
	verticalScrollBar()->setValue(verticalScrollBar()->value() - scroll_value);

	ev->accept();
}


void GuiWorkArea::generateSyntheticMouseEvent()
{
	int const e_y = d->synthetic_mouse_event_.cmd.y();
	int const wh = d->buffer_view_->workHeight();
	bool const up = e_y < 0;
	bool const down = e_y > wh;

	// Set things off to generate the _next_ 'pseudo' event.
	int step = 50;
	if (d->synthetic_mouse_event_.restart_timeout) {
		// This is some magic formulae to determine the speed
		// of scrolling related to the position of the mouse.
		int time = 200;
		if (up || down) {
			int dist = up ? -e_y : e_y - wh;
			time = max(min(200, 250000 / (dist * dist)), 1) ;

			if (time < 40) {
				step = 80000 / (time * time);
				time = 40;
			}
		}
		d->synthetic_mouse_event_.timeout.setTimeout(time);
		d->synthetic_mouse_event_.timeout.start();
	}

	// Can we scroll further ?
	int const value = verticalScrollBar()->value();
	if (value == verticalScrollBar()->maximum()
		  || value == verticalScrollBar()->minimum()) {
		d->synthetic_mouse_event_.timeout.stop();
		return;
	}

	// Scroll
	if (step <= 2 * wh) {
		d->buffer_view_->scroll(up ? -step : step);
		d->buffer_view_->updateMetrics();
	} else {
		d->buffer_view_->scrollDocView(value + (up ? -step : step), false);
	}

	// In which paragraph do we have to set the cursor ?
	Cursor & cur = d->buffer_view_->cursor();
	// FIXME: we don't know how to handle math.
	Text * text = cur.text();
	if (!text)
		return;
	TextMetrics const & tm = d->buffer_view_->textMetrics(text);

	// Quit gracefully if there are no metrics, since otherwise next
	// line would crash (bug #10324).
	// This situation seems related to a (not yet understood) timing problem.
	if (tm.empty())
		return;

	pair<pit_type, const ParagraphMetrics *> pp = up ? tm.first() : tm.last();
	ParagraphMetrics const & pm = *pp.second;
	pit_type const pit = pp.first;

	if (pm.rows().empty())
		return;

	// Find the row at which we set the cursor.
	RowList::const_iterator rit = pm.rows().begin();
	RowList::const_iterator rlast = pm.rows().end();
	int yy = pm.position() - pm.ascent();
	for (--rlast; rit != rlast; ++rit) {
		int h = rit->height();
		if ((up && yy + h > 0)
			  || (!up && yy + h > wh - defaultRowHeight()))
			break;
		yy += h;
	}

	// Find the position of the cursor
	bool bound;
	int x = d->synthetic_mouse_event_.cmd.x();
	pos_type const pos = tm.getPosNearX(*rit, x, bound);

	// Set the cursor
	cur.pit() = pit;
	cur.pos() = pos;
	cur.boundary(bound);

	d->buffer_view_->buffer().changed(false);
	return;
}


// CompressorProxy adapted from Kuba Ober https://stackoverflow.com/a/21006207
CompressorProxy::CompressorProxy(GuiWorkArea * wa) : QObject(wa), flag_(false)
{
	qRegisterMetaType<KeySymbol>("KeySymbol");
	qRegisterMetaType<KeyModifier>("KeyModifier");
	connect(wa, SIGNAL(compressKeySym(KeySymbol, KeyModifier, bool)),
		this, SLOT(slot(KeySymbol, KeyModifier, bool)),
	        Qt::QueuedConnection);
	connect(this, SIGNAL(signal(KeySymbol, KeyModifier)),
		wa, SLOT(processKeySym(KeySymbol, KeyModifier)));
}


bool CompressorProxy::emitCheck(bool isAutoRepeat)
{
	flag_ = true;
	if (isAutoRepeat)
		QCoreApplication::sendPostedEvents(this, QEvent::MetaCall); // recurse
	bool result = flag_;
	flag_ = false;
	return result;
}


void CompressorProxy::slot(KeySymbol sym, KeyModifier mod, bool isAutoRepeat)
{
	if (emitCheck(isAutoRepeat))
		Q_EMIT signal(sym, mod);
	else
		LYXERR(Debug::KEY, "system is busy: autoRepeat key event ignored");
}


void GuiWorkArea::keyPressEvent(QKeyEvent * ev)
{
	// this is also called for ShortcutOverride events. In this case, one must
	// not act but simply accept the event explicitly.
	bool const act = (ev->type() != QEvent::ShortcutOverride);

	// Do not process here some keys if dialog_mode_ is set
	bool const for_dialog_mode = d->dialog_mode_
		&& (ev->modifiers() == Qt::NoModifier
		    || ev->modifiers() == Qt::ShiftModifier)
		&& (ev->key() == Qt::Key_Escape
		    || ev->key() == Qt::Key_Enter
		    || ev->key() == Qt::Key_Return);
	// also do not use autoRepeat to input shortcuts
	bool const autoRepeat = ev->isAutoRepeat();

	if (for_dialog_mode || (!act && autoRepeat)) {
		ev->ignore();
		return;
	}

	// intercept some keys if completion popup is visible
	if (d->completer_->popupVisible()) {
		switch (ev->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if (act)
				d->completer_->activate();
			ev->accept();
			return;
		}
	}

	KeyModifier const m = q_key_state(ev->modifiers());

	if (act && lyxerr.debugging(Debug::KEY)) {
		std::string str;
		if (m & ShiftModifier)
			str += "Shift-";
		if (m & ControlModifier)
			str += "Control-";
		if (m & AltModifier)
			str += "Alt-";
		if (m & MetaModifier)
			str += "Meta-";
		LYXERR(Debug::KEY, " count: " << ev->count() << " text: " << ev->text()
		       << " isAutoRepeat: " << ev->isAutoRepeat() << " key: " << ev->key()
		       << " keyState: " << str);
	}

	KeySymbol sym;
	setKeySymbol(&sym, ev);
	if (sym.isOK()) {
		if (act) {
			Q_EMIT compressKeySym(sym, m, autoRepeat);
			ev->accept();
		} else
			// here, !autoRepeat, as determined at the beginning
			ev->setAccepted(queryKeySym(sym, m));
	} else {
		ev->ignore();
	}
}


void GuiWorkArea::doubleClickTimeout()
{
	d->dc_event_.active = false;
}


void GuiWorkArea::mouseDoubleClickEvent(QMouseEvent * ev)
{
	d->dc_event_ = DoubleClick(ev);
	QTimer::singleShot(QApplication::doubleClickInterval(), this,
			SLOT(doubleClickTimeout()));
	FuncRequest cmd(LFUN_MOUSE_DOUBLE, ev->x(), ev->y(),
			q_button_state(ev->button()), q_key_state(ev->modifiers()));
	d->dispatch(cmd);
	ev->accept();
}


void GuiWorkArea::resizeEvent(QResizeEvent * ev)
{
	QAbstractScrollArea::resizeEvent(ev);
	d->need_resize_ = true;
	ev->accept();
}


void GuiWorkArea::Private::paintPreeditText(GuiPainter & pain)
{
	if (preedit_string_.empty())
		return;

	// FIXME: shall we use real_current_font here? (see #10478)
	FontInfo const font = buffer_view_->cursor().getFont().fontInfo();
	FontMetrics const & fm = theFontMetrics(font);
	int const height = fm.maxHeight();
	int cur_x = caret_->rect().left();
	int cur_y = caret_->rect().bottom();

	// get attributes of input method cursor.
	// cursor_pos : cursor position in preedit string.
	size_t cursor_pos = 0;
	bool cursor_is_visible = false;
	for (auto const & attr : preedit_attr_) {
		if (attr.type == QInputMethodEvent::Cursor) {
			cursor_pos = attr.start;
			cursor_is_visible = attr.length != 0;
			break;
		}
	}

	size_t const preedit_length = preedit_string_.length();

	// get position of selection in input method.
	// FIXME: isn't there a way to do this simplier?
	// rStart : cursor position in selected string in IM.
	size_t rStart = 0;
	// rLength : selected string length in IM.
	size_t rLength = 0;
	if (cursor_pos < preedit_length) {
		for (auto const & attr : preedit_attr_) {
			if (attr.type == QInputMethodEvent::TextFormat) {
				if (attr.start <= int(cursor_pos)
					&& int(cursor_pos) < attr.start + attr.length) {
						rStart = attr.start;
						rLength = attr.length;
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
		char_type const typed_char = preedit_string_[pos];
		// reset preedit string style
		ps = Painter::preedit_default;

		// if we reached the right extremity of the screen, go to next line.
		if (cur_x + fm.width(typed_char) > p->viewport()->width() - right_margin) {
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
}


void GuiWorkArea::Private::resetScreen()
{
	if (use_backingstore_) {
		int const pr = p->pixelRatio();
		screen_ = QImage(static_cast<int>(pr * p->viewport()->width()),
		                 static_cast<int>(pr * p->viewport()->height()),
		                 QImage::Format_ARGB32_Premultiplied);
#  if QT_VERSION >= 0x050000
		screen_.setDevicePixelRatio(pr);
#  endif
	}
}


QPaintDevice * GuiWorkArea::Private::screenDevice()
{
	if (use_backingstore_)
		return &screen_;
	else
		return p->viewport();
}


void GuiWorkArea::Private::updateScreen(QRectF const & rc)
{
	if (use_backingstore_) {
		QPainter qpain(p->viewport());
		double const pr = p->pixelRatio();
		QRectF const rcs = QRectF(rc.x() * pr, rc.y() * pr,
		                          rc.width() * pr, rc.height() * pr);
		qpain.drawImage(rc, screen_, rcs);
	}
}


void GuiWorkArea::paintEvent(QPaintEvent * ev)
{
	// Do not trigger the painting machinery if we are not ready (see
	// bug #10989). The second test triggers when in the middle of a
	// dispatch operation.
	if (view().busy() || d->buffer_view_->buffer().undo().activeUndoGroup()) {
		// Since macOS has turned the screen black at this point, our
		// backing store has to be copied to screen (this is a no-op
		// except on macOS).
		d->updateScreen(ev->rect());
		// Ignore this paint event, but request a new one for later.
		viewport()->update(ev->rect());
		ev->accept();
		return;
	}

	// LYXERR(Debug::PAINTING, "paintEvent begin: x: " << rc.x()
	//	<< " y: " << rc.y() << " w: " << rc.width() << " h: " << rc.height());

	if (d->need_resize_ || pixelRatio() != d->last_pixel_ratio_) {
		d->resetScreen();
		d->resizeBufferView();
	}

	d->last_pixel_ratio_ = pixelRatio();

	GuiPainter pain(d->screenDevice(), pixelRatio());

	d->buffer_view_->draw(pain, d->caret_visible_);

	// The preedit text, if needed
	d->paintPreeditText(pain);

	// and the caret
	if (d->caret_visible_)
		d->caret_->draw(pain, d->buffer_view_->horizScrollOffset());

	d->updateScreen(ev->rect());

	ev->accept();
}


void GuiWorkArea::inputMethodEvent(QInputMethodEvent * e)
{
	LYXERR(Debug::KEY, "preeditString: " << e->preeditString()
		   << " commitString: " << e->commitString());

	// insert the processed text in the document (handles undo)
	if (!e->commitString().isEmpty()) {
		FuncRequest cmd(LFUN_SELF_INSERT,
		                qstring_to_ucs4(e->commitString()),
		                FuncRequest::KEYBOARD);
		dispatch(cmd);
		// FIXME: this is supposed to remove traces from preedit
		// string. Can we avoid calling it explicitely?
		d->buffer_view_->updateMetrics();
	}

	// Hide the caret during the test transformation.
	if (e->preeditString().isEmpty())
		startBlinkingCaret();
	else
		stopBlinkingCaret();

	if (d->preedit_string_.empty() && e->preeditString().isEmpty()) {
		// Nothing to do
		e->accept();
		return;
	}

	// The preedit text and its attributes will be used in paintPreeditText
	d->preedit_string_ = qstring_to_ucs4(e->preeditString());
	d->preedit_attr_ = e->attributes();


	// redraw area of preedit string.
	int height = d->caret_->rect().height();
	int cur_y = d->caret_->rect().bottom();
	viewport()->update(0, cur_y - height, viewport()->width(),
		(height + 1) * d->preedit_lines_);

	if (d->preedit_string_.empty()) {
		d->preedit_lines_ = 1;
		e->accept();
		return;
	}

	// Don't forget to accept the event!
	e->accept();
}


QVariant GuiWorkArea::inputMethodQuery(Qt::InputMethodQuery query) const
{
	QRect cur_r(0, 0, 0, 0);
	switch (query) {
		// this is the CJK-specific composition window position and
		// the context menu position when the menu key is pressed.
		case Qt::ImMicroFocus:
			cur_r = d->caret_->rect();
			if (d->preedit_lines_ != 1)
				cur_r.moveLeft(10);
			cur_r.moveBottom(cur_r.bottom()
				+ cur_r.height() * (d->preedit_lines_ - 1));
			// return lower right of caret in LyX.
			return cur_r;
		default:
			return QWidget::inputMethodQuery(query);
	}
}


void GuiWorkArea::updateWindowTitle()
{
	Buffer const & buf = bufferView().buffer();
	if (buf.fileName() != d->file_name_
	    || buf.params().shell_escape != d->shell_escape_
	    || buf.hasReadonlyFlag() != d->read_only_
	    || buf.lyxvc().vcstatus() != d->vc_status_
	    || buf.isClean() != d->clean_
	    || buf.notifiesExternalModification() != d->externally_modified_) {
		d->file_name_ = buf.fileName();
		d->shell_escape_ = buf.params().shell_escape;
		d->read_only_ = buf.hasReadonlyFlag();
		d->vc_status_ = buf.lyxvc().vcstatus();
		d->clean_ = buf.isClean();
		d->externally_modified_ = buf.notifiesExternalModification();
		Q_EMIT titleChanged(this);
	}
}


bool GuiWorkArea::isFullScreen() const
{
	return d->lyx_view_ && d->lyx_view_->isFullScreen();
}


bool GuiWorkArea::inDialogMode() const
{
	return d->dialog_mode_;
}


void GuiWorkArea::setDialogMode(bool mode)
{
	d->dialog_mode_ = mode;
}


GuiCompleter & GuiWorkArea::completer()
{
	return *d->completer_;
}

GuiView const & GuiWorkArea::view() const
{
	return *d->lyx_view_;
}


GuiView & GuiWorkArea::view()
{
	return *d->lyx_view_;
}

////////////////////////////////////////////////////////////////////
//
// EmbeddedWorkArea
//
////////////////////////////////////////////////////////////////////


EmbeddedWorkArea::EmbeddedWorkArea(QWidget * w): GuiWorkArea(w)
{
	support::TempFile tempfile("embedded.internal");
	tempfile.setAutoRemove(false);
	buffer_ = theBufferList().newInternalBuffer(tempfile.name().absFileName());
	buffer_->setUnnamed(true);
	buffer_->setFullyLoaded(true);
	setBuffer(*buffer_);
	setDialogMode(true);
}


EmbeddedWorkArea::~EmbeddedWorkArea()
{
	// No need to destroy buffer and bufferview here, because it is done
	// in theBufferList() destruction loop at application exit
}


void EmbeddedWorkArea::closeEvent(QCloseEvent * ev)
{
	disable();
	GuiWorkArea::closeEvent(ev);
}


void EmbeddedWorkArea::hideEvent(QHideEvent * ev)
{
	disable();
	GuiWorkArea::hideEvent(ev);
}


QSize EmbeddedWorkArea::sizeHint () const
{
	// FIXME(?):
	// GuiWorkArea sets the size to the screen's viewport
	// by returning a value this gets overridden
	// EmbeddedWorkArea is now sized to fit in the layout
	// of the parent, and has a minimum size set in GuiWorkArea
	// which is what we return here
	return QSize(100, 70);
}


void EmbeddedWorkArea::disable()
{
	stopBlinkingCaret();
	if (view().currentWorkArea() != this)
		return;
	// No problem if currentMainWorkArea() is 0 (setCurrentWorkArea()
	// tolerates it and shows the background logo), what happens if
	// an EmbeddedWorkArea is closed after closing all document WAs
	view().setCurrentWorkArea(view().currentMainWorkArea());
}

////////////////////////////////////////////////////////////////////
//
// TabWorkArea
//
////////////////////////////////////////////////////////////////////

#ifdef Q_OS_MAC
class NoTabFrameMacStyle : public QProxyStyle {
public:
	///
	QRect subElementRect(SubElement element, const QStyleOption * option,
			     const QWidget * widget = 0) const
	{
		QRect rect = QProxyStyle::subElementRect(element, option, widget);
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
	: QTabWidget(parent), clicked_tab_(-1), midpressed_tab_(-1)
{
#ifdef Q_OS_MAC
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

	closeBufferButton = new QToolButton(this);
	closeBufferButton->setPalette(pal);
	// FIXME: rename the icon to closebuffer.png
	closeBufferButton->setIcon(QIcon(getPixmap("images/", "closetab", "svgz,png")));
	closeBufferButton->setText("Close File");
	closeBufferButton->setAutoRaise(true);
	closeBufferButton->setCursor(Qt::ArrowCursor);
	closeBufferButton->setToolTip(qt_("Close File"));
	closeBufferButton->setEnabled(true);
	QObject::connect(closeBufferButton, SIGNAL(clicked()),
		this, SLOT(closeCurrentBuffer()));
	setCornerWidget(closeBufferButton, Qt::TopRightCorner);

	// set TabBar behaviour
	QTabBar * tb = tabBar();
	tb->setTabsClosable(!lyxrc.single_close_tab_button);
	tb->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
	tb->setElideMode(Qt::ElideNone);
	// allow dragging tabs
	tb->setMovable(true);
	// make us responsible for the context menu of the tabbar
	tb->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tb, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(showContextMenu(const QPoint &)));
	connect(tb, SIGNAL(tabCloseRequested(int)),
	        this, SLOT(closeTab(int)));

	setUsesScrollButtons(true);
}


void TabWorkArea::mousePressEvent(QMouseEvent *me)
{
	if (me->button() == Qt::MidButton)
		midpressed_tab_ = tabBar()->tabAt(me->pos());
	else
		QTabWidget::mousePressEvent(me);
}


void TabWorkArea::mouseReleaseEvent(QMouseEvent *me)
{
	if (me->button() == Qt::MidButton) {
		int const midreleased_tab = tabBar()->tabAt(me->pos());
		if (midpressed_tab_ == midreleased_tab && posIsTab(me->pos()))
			closeTab(midreleased_tab);
	} else
		QTabWidget::mouseReleaseEvent(me);
}


void TabWorkArea::paintEvent(QPaintEvent * event)
{
	if (tabBar()->isVisible()) {
		QTabWidget::paintEvent(event);
	} else {
		// Prevent the selected tab to influence the
		// painting of the frame of the tab widget.
		// This is needed for gtk style in Qt.
		QStylePainter p(this);
#if QT_VERSION < 0x050000
		QStyleOptionTabWidgetFrameV2 opt;
#else
		QStyleOptionTabWidgetFrame opt;
#endif
		initStyleOption(&opt);
		opt.rect = style()->subElementRect(QStyle::SE_TabWidgetTabPane,
			&opt, this);
		opt.selectedTabRect = QRect();
		p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
	}
}


bool TabWorkArea::posIsTab(QPoint position)
{
	// tabAt returns -1 if tab does not covers position
	return tabBar()->tabAt(position) > -1;
}


void TabWorkArea::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (event->button() != Qt::LeftButton)
		return;

	// this code chunk is unnecessary because it seems the event only makes
	// it this far if it is not on a tab. I'm not sure why this is (maybe
	// it is handled and ended in DragTabBar?), and thus I'm not sure if
	// this is true in all cases and if it will be true in the future so I
	// leave this code for now. (skostysh, 2016-07-21)
	//
	// return early if double click on existing tabs
	if (posIsTab(event->pos()))
		return;

	dispatch(FuncRequest(LFUN_BUFFER_NEW));
}


void TabWorkArea::setFullScreen(bool full_screen)
{
	for (int i = 0; i != count(); ++i) {
		if (GuiWorkArea * wa = workArea(i))
			wa->setFullScreen(full_screen);
	}

	if (lyxrc.full_screen_tabbar)
		showBar(!full_screen && count() > 1);
	else
		showBar(count() > 1);
}


void TabWorkArea::showBar(bool show)
{
	tabBar()->setEnabled(show);
	tabBar()->setVisible(show);
	closeBufferButton->setVisible(show && lyxrc.single_close_tab_button);
	setTabsClosable(!lyxrc.single_close_tab_button);
}


GuiWorkAreaContainer * TabWorkArea::widget(int index) const
{
	QWidget * w = QTabWidget::widget(index);
	if (!w)
		return nullptr;
	GuiWorkAreaContainer * wac = dynamic_cast<GuiWorkAreaContainer *>(w);
	LATTEST(wac);
	return wac;
}


GuiWorkAreaContainer * TabWorkArea::currentWidget() const
{
	return widget(currentIndex());
}


GuiWorkArea * TabWorkArea::workArea(int index) const
{
	GuiWorkAreaContainer * w = widget(index);
	if (!w)
		return nullptr;
	return w->workArea();
}


GuiWorkArea * TabWorkArea::currentWorkArea() const
{
	return workArea(currentIndex());
}


GuiWorkArea * TabWorkArea::workArea(Buffer & buffer) const
{
	// FIXME: this method doesn't work if we have more than one work area
	// showing the same buffer.
	for (int i = 0; i != count(); ++i) {
		GuiWorkArea * wa = workArea(i);
		LASSERT(wa, return 0);
		if (&wa->bufferView().buffer() == &buffer)
			return wa;
	}
	return 0;
}


void TabWorkArea::closeAll()
{
	while (count()) {
		QWidget * wac = widget(0);
		LASSERT(wac, return);
		removeTab(0);
		delete wac;
	}
}


int TabWorkArea::indexOfWorkArea(GuiWorkArea * w) const
{
	for (int index = 0; index < count(); ++index)
		if (workArea(index) == w)
			return index;
	return -1;
}


bool TabWorkArea::setCurrentWorkArea(GuiWorkArea * work_area)
{
	LASSERT(work_area, return false);
	int index = indexOfWorkArea(work_area);
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
	GuiWorkAreaContainer * wac = new GuiWorkAreaContainer(wa);
	wa->setUpdatesEnabled(false);
	// Hide tabbar if there's no tab (avoid a resize and a flashing tabbar
	// when hiding it again below).
	if (!(currentWorkArea() && currentWorkArea()->isFullScreen()))
		showBar(count() > 0);
	addTab(wac, wa->windowTitle());
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
	int index = indexOfWorkArea(work_area);
	if (index == -1)
		return false;

	work_area->setUpdatesEnabled(false);
	QWidget * wac = widget(index);
	removeTab(index);
	delete wac;

	if (count()) {
		// make sure the next work area is enabled.
		currentWidget()->setUpdatesEnabled(true);
		if (currentWorkArea() && currentWorkArea()->isFullScreen())
			setFullScreen(true);
		else
			// Show tabbar only if there's more than one tab.
			showBar(count() > 1);
	} else
		lastWorkAreaRemoved();

	updateTabTexts();

	return true;
}


void TabWorkArea::on_currentTabChanged(int i)
{
	// returns e.g. on application destruction
	if (i == -1)
		return;
	GuiWorkArea * wa = workArea(i);
	LASSERT(wa, return);
	wa->setUpdatesEnabled(true);
	wa->scheduleRedraw(true);
	wa->setFocus();
	///
	currentWorkAreaChanged(wa);

	LYXERR(Debug::GUI, "currentTabChanged " << i
		<< " File: " << wa->bufferView().buffer().absFileName());
}


void TabWorkArea::closeCurrentBuffer()
{
	GuiWorkArea * wa;
	if (clicked_tab_ == -1)
		wa = currentWorkArea();
	else {
		wa = workArea(clicked_tab_);
		LASSERT(wa, return);
	}
	wa->view().closeWorkArea(wa);
}


void TabWorkArea::hideCurrentTab()
{
	GuiWorkArea * wa;
	if (clicked_tab_ == -1)
		wa = currentWorkArea();
	else {
		wa = workArea(clicked_tab_);
		LASSERT(wa, return);
	}
	wa->view().hideWorkArea(wa);
}


void TabWorkArea::closeTab(int index)
{
	on_currentTabChanged(index);
	GuiWorkArea * wa;
	if (index == -1)
		wa = currentWorkArea();
	else {
		wa = workArea(index);
		LASSERT(wa, return);
	}
	wa->view().closeWorkArea(wa);
}


///
class DisplayPath {
public:
	/// make vector happy
	DisplayPath() : tab_(-1), dottedPrefix_(false) {}
	///
	DisplayPath(int tab, FileName const & filename)
		: tab_(tab)
	{
		filename_ = (filename.extension() == "lyx") ?
			toqstr(filename.onlyFileNameWithoutExt())
			: toqstr(filename.onlyFileName());
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
				prefix_ += ellipsisSlash_;
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
		return prefix_ + (dots ? ellipsisSlash_ : "") + filename_;
	}
	///
	QString forecastPathString() const
	{
		if (postfix_.count() == 0)
			return displayString();

		return prefix_
			+ (dottedPrefix_ ? ellipsisSlash_ : "")
			+ postfix_.front() + "/";
	}
	///
	bool final() const { return postfix_.empty(); }
	///
	int tab() const { return tab_; }

private:
	/// ".../"
	static QString const ellipsisSlash_;
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


QString const DisplayPath::ellipsisSlash_ = QString(QChar(0x2026)) + "/";


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
		GuiWorkArea * i_wa = workArea(i);
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

			// We found a non-atomic segment
			// We know that segStart <= it < next <= paths.end().
			// The assertion below tells coverity about it.
			LATTEST(segStart != paths.end());
			QString dspString = segStart->forecastPathString();
			LYXERR(Debug::GUI, "first forecast found for "
			       << segStart->abs() << " => " << dspString);
			It sit = segStart;
			++sit;
			// Shift path segments and hope for the best
			// that it makes the path more unique.
			somethingChanged = true;
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
		int const tab_index = it->tab();
		Buffer const & buf = workArea(tab_index)->bufferView().buffer();
		QString tab_text = it->displayString().replace("&", "&&");
		if (!buf.fileName().empty() && !buf.isClean())
			tab_text += "*";
		QString tab_tooltip = it->abs();
		if (buf.hasReadonlyFlag()) {
			setTabIcon(tab_index, QIcon(getPixmap("images/", "emblem-readonly", "svgz,png")));
			tab_tooltip = qt_("%1 (read only)").arg(tab_tooltip);
		} else
			setTabIcon(tab_index, QIcon());
		if (buf.notifiesExternalModification()) {
			QString const warn = qt_("%1 (modified externally)");
			tab_tooltip = warn.arg(tab_tooltip);
			tab_text += QChar(0x26a0);
		}
		setTabText(tab_index, tab_text);
		setTabToolTip(tab_index, tab_tooltip);
	}
}


void TabWorkArea::showContextMenu(const QPoint & pos)
{
	// which tab?
	clicked_tab_ = tabBar()->tabAt(pos);
	if (clicked_tab_ == -1)
		return;

	// show tab popup
	QMenu popup;
	popup.addAction(QIcon(getPixmap("images/", "hidetab", "svgz,png")),
		qt_("Hide tab"), this, SLOT(hideCurrentTab()));
	popup.addAction(QIcon(getPixmap("images/", "closetab", "svgz,png")),
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


GuiWorkAreaContainer::GuiWorkAreaContainer(GuiWorkArea * wa, QWidget * parent)
	: QWidget(parent), wa_(wa)
{
	LASSERT(wa, return);
	Ui::WorkAreaUi::setupUi(this);
	layout()->addWidget(wa);
	connect(wa, SIGNAL(titleChanged(GuiWorkArea *)),
	        this, SLOT(updateDisplay()));
	connect(reloadPB, SIGNAL(clicked()), this, SLOT(reload()));
	connect(ignorePB, SIGNAL(clicked()), this, SLOT(ignore()));
	setMessageColour({notificationFrame}, {reloadPB, ignorePB});
	updateDisplay();
}


void GuiWorkAreaContainer::updateDisplay()
{
	Buffer const & buf = wa_->bufferView().buffer();
	notificationFrame->setHidden(!buf.notifiesExternalModification());
	QString const label = qt_("<b>The file %1 changed on disk.</b>")
		.arg(toqstr(buf.fileName().displayName()));
	externalModificationLabel->setText(label);
}


void GuiWorkAreaContainer::dispatch(FuncRequest f) const
{
	lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
	                          wa_->bufferView().buffer().absFileName()));
	lyx::dispatch(f);
}


void GuiWorkAreaContainer::reload() const
{
	dispatch(FuncRequest(LFUN_BUFFER_RELOAD));
}


void GuiWorkAreaContainer::ignore() const
{
	dispatch(FuncRequest(LFUN_BUFFER_EXTERNAL_MODIFICATION_CLEAR));
}


void GuiWorkAreaContainer::mouseDoubleClickEvent(QMouseEvent * event)
{
	// prevent TabWorkArea from opening a new buffer on double click
	event->accept();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiWorkArea.cpp"
