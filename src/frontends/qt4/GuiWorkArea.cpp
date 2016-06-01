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
#include "Menus.h"

#include "Buffer.h"
#include "BufferList.h"
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
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "qt_helpers.h"
#include "Text.h"
#include "TextMetrics.h"
#include "version.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/FileName.h"
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
#include <QPixmapCache>
#include <QScrollBar>
#include <QStyleOption>
#include <QStylePainter>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QMenuBar>

#include "support/bind.h"

#include <cmath>

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
	CursorWidget() : rtl_(false), l_shape_(false), completable_(false),
		show_(false), x_(0), cursor_width_(0)
	{
		recomputeWidth();
	}

	void draw(QPainter & painter)
	{
		if (!show_ || !rect_.isValid())
			return;

		int y = rect_.top();
		int l = x_ - rect_.left();
		int r = rect_.right() - x_;
		int bot = rect_.bottom();

		// draw vertical line
		painter.fillRect(x_, y, cursor_width_, rect_.height(), color_);

		// draw RTL/LTR indication
		painter.setPen(color_);
		if (l_shape_) {
			if (rtl_)
				painter.drawLine(x_, bot, x_ - l, bot);
			else
				painter.drawLine(x_, bot, x_ + cursor_width_ + r, bot);
		}

		// draw completion triangle
		if (completable_) {
			int m = y + rect_.height() / 2;
			int d = TabIndicatorWidth - 1;
			if (rtl_) {
				painter.drawLine(x_ - 1, m - d, x_ - 1 - d, m);
				painter.drawLine(x_ - 1, m + d, x_ - 1 - d, m);
			} else {
				painter.drawLine(x_ + cursor_width_, m - d, x_ + cursor_width_ + d, m);
				painter.drawLine(x_ + cursor_width_, m + d, x_ + cursor_width_ + d, m);
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
		rect_ = QRect(x - l, y, cursor_width_ + r + l, h);
	}

	void show(bool set_show = true) { show_ = set_show; }
	void hide() { show_ = false; }
	int cursorWidth() const { return cursor_width_; }
	void recomputeWidth() {
		cursor_width_ = lyxrc.cursor_width
			? lyxrc.cursor_width 
			: 1 + int((lyxrc.zoom + 50) / 200.0);
	}

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
	
	int cursor_width_;
};


// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true)
{}


GuiWorkArea::Private::Private(GuiWorkArea * parent)
: p(parent), screen_(0), buffer_view_(0), read_only_(false), lyx_view_(0),
cursor_visible_(false), cursor_(0),
need_resize_(false), schedule_redraw_(false), preedit_lines_(1),
pixel_ratio_(1.0),
completer_(new GuiCompleter(p, p)), dialog_mode_(false)
{
}


GuiWorkArea::GuiWorkArea(QWidget * /* w */)
: d(new Private(this)) 
{
}


GuiWorkArea::GuiWorkArea(Buffer & buffer, GuiView & gv)
: d(new Private(this))
{
	setGuiView(gv);
	buffer.params().display_pixel_ratio = theGuiApp()->pixelRatio();
	setBuffer(buffer);
	init();
}


double GuiWorkArea::pixelRatio() const
{
#if QT_VERSION >= 0x050000
	return devicePixelRatio();
#else
	return 1.0;
#endif
}


void GuiWorkArea::init()
{
	// Setup the signals
	connect(&d->cursor_timeout_, SIGNAL(timeout()),
		this, SLOT(toggleCursor()));

	int const time = QApplication::cursorFlashTime() / 2;
	if (time > 0) {
		d->cursor_timeout_.setInterval(time);
		d->cursor_timeout_.start();
	} else {
		// let's initialize this just to be safe
		d->cursor_timeout_.setInterval(500);
	}

	d->resetScreen();
	// With Qt4.5 a mouse event will happen before the first paint event
	// so make sure that the buffer view has an up to date metrics.
	d->buffer_view_->resize(viewport()->width(), viewport()->height());
	d->cursor_ = new frontend::CursorWidget();
	d->cursor_->hide();

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptDrops(true);
	setMouseTracking(true);
	setMinimumSize(100, 70);
	setFrameStyle(QFrame::NoFrame);
	updateWindowTitle();

	viewport()->setAutoFillBackground(false);
	// We don't need double-buffering nor SystemBackground on
	// the viewport because we have our own backing pixmap.
	viewport()->setAttribute(Qt::WA_NoSystemBackground);

	setFocusPolicy(Qt::StrongFocus);

	d->setCursorShape(Qt::IBeamCursor);

	d->synthetic_mouse_event_.timeout.timeout.connect(
		bind(&GuiWorkArea::generateSyntheticMouseEvent,
					this));

	// Initialize the vertical Scroll Bar
	QObject::connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		this, SLOT(scrollTo(int)));

	LYXERR(Debug::GUI, "viewport width: " << viewport()->width()
		<< "  viewport height: " << viewport()->height());

	// Enables input methods for asian languages.
	// Must be set when creating custom text editing widgets.
	setAttribute(Qt::WA_InputMethodEnabled, true);

	d->dialog_mode_ = false;
}


GuiWorkArea::~GuiWorkArea()
{
	d->buffer_view_->buffer().workAreaManager().remove(this);
	delete d->screen_;
	delete d->buffer_view_;
	delete d->cursor_;
	// Completer has a QObject parent and is thus automatically destroyed.
	// See #4758.
	// delete completer_;
	delete d;
}


Qt::CursorShape GuiWorkArea::cursorShape() const
{
	return viewport()->cursor().shape();
}


void GuiWorkArea::Private::setCursorShape(Qt::CursorShape shape)
{
	p->viewport()->setCursor(shape);
}


void GuiWorkArea::Private::updateCursorShape()
{
	setCursorShape(buffer_view_->clickableInset() 
		? Qt::PointingHandCursor : Qt::IBeamCursor);
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


void GuiWorkArea::stopBlinkingCursor()
{
	d->cursor_timeout_.stop();
	d->hideCursor();
}


void GuiWorkArea::startBlinkingCursor()
{
	// do not show the cursor if the view is busy
	if (view().busy())
		return;

	Point p;
	int h = 0;
	d->buffer_view_->cursorPosAndHeight(p, h);
	// Don't start blinking if the cursor isn't on screen.
	if (!d->buffer_view_->cursorInView(p, h))
		return;

	d->showCursor();

	//we're not supposed to cache this value.
	int const time = QApplication::cursorFlashTime() / 2;
	if (time <= 0)
		return;
	d->cursor_timeout_.setInterval(time);
	d->cursor_timeout_.start();
}


void GuiWorkArea::redraw(bool update_metrics)
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

	// update cursor position, because otherwise it has to wait until
	// the blinking interval is over
	if (d->cursor_visible_) {
		d->hideCursor();
		d->showCursor();
	}

	LYXERR(Debug::WORKAREA, "WorkArea::redraw screen");
	d->updateScreen();
	update(0, 0, viewport()->width(), viewport()->height());

	/// \warning: scrollbar updating *must* be done after the BufferView is drawn
	/// because \c BufferView::updateScrollbar() is called in \c BufferView::draw().
	d->updateScrollbar();
	d->lyx_view_->updateStatusBar();

	if (lyxerr.debugging(Debug::WORKAREA))
		d->buffer_view_->coordCache().dump();

	updateWindowTitle();

	d->updateCursorShape();
}


void GuiWorkArea::processKeySym(KeySymbol const & key, KeyModifier mod)
{
	if (d->lyx_view_->isFullScreen() && d->lyx_view_->menuBar()->isVisible()
		&& lyxrc.full_screen_menubar) {
		// FIXME HACK: we should not have to do this here. See related comment
		// in GuiView::event() (QEvent::ShortcutOverride)
		d->lyx_view_->menuBar()->hide();
	}

	// In order to avoid bad surprise in the middle of an operation,
	// we better stop the blinking cursor...
	// the cursor gets restarted in GuiView::restartCursor()
	stopBlinkingCursor();
	guiApp->processKeySym(key, mod);
}


void GuiWorkArea::Private::dispatch(FuncRequest const & cmd0, KeyModifier mod)
{
	// Handle drag&drop
	if (cmd0.action() == LFUN_FILE_OPEN) {
		DispatchResult dr;
		lyx_view_->dispatch(cmd0, dr);
		return;
	}

	FuncRequest cmd;

	if (cmd0.action() == LFUN_MOUSE_PRESS) {
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
		cmd.action() != LFUN_MOUSE_MOTION || cmd.button() != mouse_button::none;

	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking cursor.
	if (notJustMovingTheMouse)
		p->stopBlinkingCursor();

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

		// Show the cursor immediately after any operation
		p->startBlinkingCursor();
	}

	updateCursorShape();
}


void GuiWorkArea::Private::resizeBufferView()
{
	// WARNING: Please don't put any code that will trigger a repaint here!
	// We are already inside a paint event.
	p->stopBlinkingCursor();
	// Warn our container (GuiView).
	p->busy(true);

	Point point;
	int h = 0;
	buffer_view_->cursorPosAndHeight(point, h);
	bool const cursor_in_view = buffer_view_->cursorInView(point, h);
	buffer_view_->resize(p->viewport()->width(), p->viewport()->height());
	if (cursor_in_view)
		buffer_view_->scrollToCursor();
	updateScreen();

	// Update scrollbars which might have changed due different
	// BufferView dimension. This is especially important when the
	// BufferView goes from zero-size to the real-size for the first time,
	// as the scrollbar paramters are then set for the first time.
	updateScrollbar();

	need_resize_ = false;
	p->busy(false);
	// Eventually, restart the cursor after the resize event.
	// We might be resizing even if the focus is on another widget so we only
	// restart the cursor if we have the focus.
	if (p->hasFocus())
		QTimer::singleShot(50, p, SLOT(startBlinkingCursor()));
}


void GuiWorkArea::Private::showCursor()
{
	if (cursor_visible_)
		return;

	Point p;
	int h = 0;
	buffer_view_->cursorPosAndHeight(p, h);
	if (!buffer_view_->cursorInView(p, h))
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

	// show cursor on screen
	Cursor & cur = buffer_view_->cursor();
	bool completable = cur.inset().showCompletionCursor()
		&& completer_->completionAvailable()
		&& !completer_->popupVisible()
		&& !completer_->inlineVisible();
	cursor_visible_ = true;
	cursor_->recomputeWidth();

	//int cur_x = buffer_view_->getPos(cur).x_;
	// We may have decided to slide the cursor row so that cursor
	// is visible.
	p.x_ -= buffer_view_->horizScrollOffset();

	showCursor(p.x_, p.y_, h, l_shape, isrtl, completable);
}


void GuiWorkArea::Private::hideCursor()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	removeCursor();
}


void GuiWorkArea::toggleCursor()
{
	if (d->cursor_visible_)
		d->hideCursor();
	else
		d->showCursor();
}


void GuiWorkArea::Private::updateScrollbar()
{
	ScrollbarParameters const & scroll_ = buffer_view_->scrollbarParameters();
	// WARNING: don't touch at the scrollbar value like this:
	//   verticalScrollBar()->setValue(scroll_.position);
	// because this would cause a recursive signal/slot calling with
	// GuiWorkArea::scrollTo
	p->verticalScrollBar()->setRange(scroll_.min, scroll_.max);
	p->verticalScrollBar()->setPageStep(scroll_.page_step);
	p->verticalScrollBar()->setSingleStep(scroll_.single_step);
	p->verticalScrollBar()->setSliderPosition(scroll_.position);
}


void GuiWorkArea::scrollTo(int value)
{
	stopBlinkingCursor();
	d->buffer_view_->scrollDocView(value, true);

	if (lyxrc.cursor_follows_scrollbar) {
		d->buffer_view_->setCursorFromScrollbar();
		// FIXME: let GuiView take care of those.
		d->lyx_view_->updateLayoutList();
	}
	// Show the cursor immediately after any operation.
	startBlinkingCursor();
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
		if ((ke->key() == Qt::Key_Tab && ke->modifiers() == Qt::NoModifier)
			|| (ke->key() == Qt::Key_Backtab && (
				ke->modifiers() == Qt::ShiftModifier
				|| ke->modifiers() == Qt::NoModifier))) {
			e->accept();
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
		QAbstractScrollArea::contextMenuEvent(e);
		return;
	}
	// always show mnemonics when the keyboard is used to show the context menu
	// FIXME: This should be fixed in Qt itself
	bool const keyboard = (e->reason() == QContextMenuEvent::Keyboard);
	QMenu * menu = guiApp->menus().menu(toqstr(name), *d->lyx_view_, keyboard);
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
	LYXERR(Debug::DEBUG, "GuiWorkArea::focusInEvent(): " << this << endl);
	if (d->lyx_view_->currentWorkArea() != this) {
		d->lyx_view_->setCurrentWorkArea(this);
		d->lyx_view_->currentWorkArea()->bufferView().buffer().updateBuffer();
	}

	startBlinkingCursor();
	QAbstractScrollArea::focusInEvent(e);
}


void GuiWorkArea::focusOutEvent(QFocusEvent * e)
{
	LYXERR(Debug::DEBUG, "GuiWorkArea::focusOutEvent(): " << this << endl);
	stopBlinkingCursor();
	QAbstractScrollArea::focusOutEvent(e);
}


void GuiWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (d->dc_event_.active && d->dc_event_ == *e) {
		d->dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE, e->x(), e->y(),
			q_button_state(e->button()));
		d->dispatch(cmd);
		e->accept();
		return;
	}

#if (QT_VERSION < 0x050000) && !defined(__HAIKU__)
	inputContext()->reset();
#endif

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
		q_button_state(e->button()));
	d->dispatch(cmd, q_key_state(e->modifiers()));

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
			      q_button_state(e->button()));
	d->dispatch(cmd);
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
	double const delta = ev->delta() / 120.0;
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
	int scroll_value = lines > page_step
		? page_step : lines * verticalScrollBar()->singleStep();

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
	// FIXME: we don't know howto handle math.
	Text * text = cur.text();
	if (!text)
		return;
	TextMetrics const & tm = d->buffer_view_->textMetrics(text);

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


void GuiWorkArea::keyPressEvent(QKeyEvent * ev)
{
	// Do not process here some keys if dialog_mode_ is set
	if (d->dialog_mode_
		&& (ev->modifiers() == Qt::NoModifier
		    || ev->modifiers() == Qt::ShiftModifier)
		&& (ev->key() == Qt::Key_Escape
		    || ev->key() == Qt::Key_Enter
		    || ev->key() == Qt::Key_Return)
	    ) {
		ev->ignore();
		return;
	}

	// intercept some keys if completion popup is visible
	if (d->completer_->popupVisible()) {
		switch (ev->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			d->completer_->activate();
			ev->accept();
			return;
		}
	}

	// do nothing if there are other events
	// (the auto repeated events come too fast)
	// it looks like this is only needed on X11
#if defined(Q_WS_X11) || defined(QPA_XCB)
	if (qApp->hasPendingEvents() && ev->isAutoRepeat()) {
		switch (ev->key()) {
		case Qt::Key_PageDown:
		case Qt::Key_PageUp:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Up:
		case Qt::Key_Down:
			LYXERR(Debug::KEY, "system is busy: scroll key event ignored");
			ev->ignore();
			return;
		}
	}
#endif

	KeyModifier m = q_key_state(ev->modifiers());

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

	KeySymbol sym;
	setKeySymbol(&sym, ev);
	if (sym.isOK()) {
		processKeySym(sym, q_key_state(ev->modifiers()));
		ev->accept();
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
	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
			ev->x(), ev->y(),
			q_button_state(ev->button()));
	d->dispatch(cmd);
	ev->accept();
}


void GuiWorkArea::resizeEvent(QResizeEvent * ev)
{
	QAbstractScrollArea::resizeEvent(ev);
	d->need_resize_ = true;
	ev->accept();
}


void GuiWorkArea::Private::update(int x, int y, int w, int h)
{
	p->viewport()->update(x, y, w, h);
}


void GuiWorkArea::paintEvent(QPaintEvent * ev)
{
	QRectF const rc = ev->rect();
	// LYXERR(Debug::PAINTING, "paintEvent begin: x: " << rc.x()
	//	<< " y: " << rc.y() << " w: " << rc.width() << " h: " << rc.height());

	if (d->needResize()) {
		d->resetScreen();
		d->resizeBufferView();
		if (d->cursor_visible_) {
			d->hideCursor();
			d->showCursor();
		}
	}

	QPainter pain(viewport());
	double const pr = pixelRatio();
	QRectF const rcs = QRectF(rc.x() * pr, rc.y() * pr, rc.width() * pr, rc.height() * pr);

	if (lyxrc.use_qimage) {
		QImage const & image = static_cast<QImage const &>(*d->screen_);
		pain.drawImage(rc, image, rcs);
	} else {
		QPixmap const & pixmap = static_cast<QPixmap const &>(*d->screen_);
		pain.drawPixmap(rc, pixmap, rcs);
	}
	d->cursor_->draw(pain);
	ev->accept();
}


void GuiWorkArea::Private::updateScreen()
{
	GuiPainter pain(screen_, p->pixelRatio());
	buffer_view_->draw(pain);
}


void GuiWorkArea::Private::showCursor(int x, int y, int h,
	bool l_shape, bool rtl, bool completable)
{
	if (schedule_redraw_) {
		// This happens when a graphic conversion is finished. As we don't know
		// the size of the new graphics, it's better the update everything.
		// We can't use redraw() here because this would trigger a infinite
		// recursive loop with showCursor().
		buffer_view_->resize(p->viewport()->width(), p->viewport()->height());
		updateScreen();
		updateScrollbar();
		p->viewport()->update(QRect(0, 0, p->viewport()->width(), p->viewport()->height()));
		schedule_redraw_ = false;
		// Show the cursor immediately after the update.
		hideCursor();
		p->toggleCursor();
		return;
	}

	cursor_->update(x, y, h, l_shape, rtl, completable);
	cursor_->show();
	p->viewport()->update(cursor_->rect());
}


void GuiWorkArea::Private::removeCursor()
{
	cursor_->hide();
	//if (!qApp->focusWidget())
		p->viewport()->update(cursor_->rect());
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
	// FIXME THREAD
	// We could have more than one work area, right?
	static bool last_width = false;
	if (!last_width && preedit_string.empty()) {
		// if last_width is last length of preedit string.
		e->accept();
		return;
	}

	GuiPainter pain(d->screen_, pixelRatio());
	d->buffer_view_->updateMetrics();
	d->buffer_view_->draw(pain);
	FontInfo font = d->buffer_view_->cursor().getFont().fontInfo();
	FontMetrics const & fm = theFontMetrics(font);
	int height = fm.maxHeight();
	int cur_x = d->cursor_->rect().left();
	int cur_y = d->cursor_->rect().bottom();

	// redraw area of preedit string.
	update(0, cur_y - height, viewport()->width(),
		(height + 1) * d->preedit_lines_);

	if (preedit_string.empty()) {
		last_width = false;
		d->preedit_lines_ = 1;
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

	int const right_margin = d->buffer_view_->rightMargin();
	Painter::preedit_style ps;
	// Most often there would be only one line:
	d->preedit_lines_ = 1;
	for (size_t pos = 0; pos != preedit_length; ++pos) {
		char_type const typed_char = preedit_string[pos];
		// reset preedit string style
		ps = Painter::preedit_default;

		// if we reached the right extremity of the screen, go to next line.
		if (cur_x + fm.width(typed_char) > viewport()->width() - right_margin) {
			cur_x = right_margin;
			cur_y += height + 1;
			++d->preedit_lines_;
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
	update(0, cur_y - d->preedit_lines_*height, viewport()->width(),
		(height + 1) * d->preedit_lines_);

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
			cur_r = d->cursor_->rect();
			if (d->preedit_lines_ != 1)
				cur_r.moveLeft(10);
			cur_r.moveBottom(cur_r.bottom()
				+ cur_r.height() * (d->preedit_lines_ - 1));
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

	Buffer const & buf = d->buffer_view_->buffer();
	FileName const file_name = buf.fileName();
	if (!file_name.empty()) {
		maximize_title = file_name.displayName(130);
		minimize_title = from_utf8(file_name.onlyFileName());
		if (buf.lyxvc().inUse()) {
			if (buf.lyxvc().locking())
				maximize_title +=  _(" (version control, locking)");
			else
				maximize_title +=  _(" (version control)");
		}
		if (!buf.isClean()) {
			maximize_title += _(" (changed)");
			minimize_title += char_type('*');
		}
		if (buf.isReadonly())
			maximize_title += _(" (read only)");
	}

	QString const new_title = toqstr(maximize_title);
	if (new_title != windowTitle()) {
		QWidget::setWindowTitle(new_title);
		QWidget::setWindowIconText(toqstr(minimize_title));
		titleChanged(this);
	}
}


bool GuiWorkArea::isFullScreen() const
{
	return d->lyx_view_ && d->lyx_view_->isFullScreen();
}


void GuiWorkArea::scheduleRedraw()
{
	d->schedule_redraw_ = true;
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
	stopBlinkingCursor();
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
	: QTabWidget(parent), clicked_tab_(-1)
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
	connect(tb, SIGNAL(tabCloseRequested(int)),
		this, SLOT(closeTab(int)));

	setUsesScrollButtons(true);
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


void TabWorkArea::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (event->button() != Qt::LeftButton)
		return;

	// return early if double click on existing tabs
	for (int i = 0; i < count(); ++i)
		if (tabBar()->tabRect(i).contains(event->pos()))
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


GuiWorkArea * TabWorkArea::currentWorkArea()
{
	if (count() == 0)
		return 0;

	GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(currentWidget());
	LATTEST(wa);
	return wa;
}


GuiWorkArea * TabWorkArea::workArea(int index)
{
	return dynamic_cast<GuiWorkArea *>(widget(index));
}


GuiWorkArea * TabWorkArea::workArea(Buffer & buffer)
{
	// FIXME: this method doesn't work if we have more than work area
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
		GuiWorkArea * wa = workArea(0);
		LASSERT(wa, return);
		removeTab(0);
		delete wa;
	}
}


bool TabWorkArea::setCurrentWorkArea(GuiWorkArea * work_area)
{
	LASSERT(work_area, return false);
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
	wa->redraw(true);
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
	DisplayPath() {}
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
		int const tab_index = it->tab();
		Buffer const & buf = workArea(tab_index)->bufferView().buffer();
		QString tab_text = it->displayString().replace("&", "&&");
		if (!buf.fileName().empty() && !buf.isClean())
			tab_text += "*";
		QString tab_tooltip = it->abs();
		if (buf.isReadonly()) {
			setTabIcon(tab_index, QIcon(getPixmap("images/", "emblem-readonly", "svgz,png")));
			tab_tooltip = qt_("%1 (read only)").arg(it->abs());
		} else
			setTabIcon(tab_index, QIcon());
		setTabText(tab_index, tab_text);
		setTabToolTip(tab_index, tab_tooltip);
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


DragTabBar::DragTabBar(QWidget* parent)
	: QTabBar(parent)
{
	setAcceptDrops(true);
	setTabsClosable(!lyxrc.single_close_tab_button);
}


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

	// get tab pixmap as cursor
	QRect r = tabRect(tab);
	QPixmap pixmap(r.size());
	render(&pixmap, - r.topLeft());
	drag->setPixmap(pixmap);
	drag->exec();
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

#include "moc_GuiWorkArea.cpp"
