/**
 * \file GuiWorkArea.C
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

#include "Application.h"
#include "ColorCache.h"
#include "QLPainter.h"
#include "QLyXKeySym.h"
#include "qt_helpers.h"

#include "LyXView.h"

#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"

#include "support/os.h"

#include <QLayout>
#include <QMainWindow>
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QPainter>
#include <QScrollBar>

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

// Abdel (26/06/2006):
// On windows-XP the UserGuide PageDown scroll test is faster without event pruning (16 s)
// than with it (23 s).
#ifdef Q_WS_WIN
 #define USE_EVENT_PRUNING 0
#else
 #define USE_EVENT_PRUNING 0
#endif

using std::endl;
using std::string;

namespace os = lyx::support::os;

namespace {

/// return the LyX key state from Qt's
key_modifier::state q_key_state(Qt::KeyboardModifiers state)
{
	key_modifier::state k = key_modifier::none;
	if (state & Qt::ControlModifier)
		k |= key_modifier::ctrl;
	if (state & Qt::ShiftModifier)
		k |= key_modifier::shift;
	if (state & Qt::AltModifier || state & Qt::MetaModifier)
		k |= key_modifier::alt;
	return k;
}


/// return the LyX mouse button state from Qt's
mouse_button::state q_button_state(Qt::MouseButton button)
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
mouse_button::state q_motion_state(Qt::MouseButton state)
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

} // namespace anon

namespace lyx {
namespace frontend {

// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true),
	  x_old(-1), y_old(-1), scrollbar_value_old(-1.0)
{}


GuiWorkArea::GuiWorkArea(int w, int h, LyXView & lyx_view)
: WorkArea(lyx_view), painter_(this)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setAcceptDrops(true);

	setMinimumSize(100, 70);

	viewport()->setAutoFillBackground(false);
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);

	viewport()->setFocusPolicy(Qt::WheelFocus);
	viewport()->setFocus();
	setFocusPolicy(Qt::WheelFocus);

	viewport()->setCursor(Qt::IBeamCursor);

	resize(w, h);

	synthetic_mouse_event_.timeout.timeout.connect(
		boost::bind(&GuiWorkArea::generateSyntheticMouseEvent,
			    this));

	// Initialize the vertical Scroll Bar
	QObject::connect(verticalScrollBar(), SIGNAL(actionTriggered(int)),
		this, SLOT(adjustViewWithScrollBar(int)));

	// PageStep only depends on the viewport height.
	verticalScrollBar()->setPageStep(viewport()->height());

	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n Area width\t" << width()
		<< "\n Area height\t" << height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< endl;

	if (USE_EVENT_PRUNING) {
		// This is the keyboard buffering stuff...
		// I don't see any need for this under windows. The keyboard is reactive
		// enough...

		if ( !QObject::connect(&step_timer_, SIGNAL(timeout()),
			this, SLOT(keyeventTimeout())) )
			lyxerr[Debug::GUI] << "ERROR: keyeventTimeout cannot connect!" << endl;

		// Start the timer, one-shot.
		step_timer_.setSingleShot(true);
		step_timer_.start(50);
	}

	// Enables input methods for asian languages.
	// Must be set when creating custom text editing widgets.
	setAttribute(Qt::WA_InputMethodEnabled, true);
}


GuiWorkArea::~GuiWorkArea()
{
}


void GuiWorkArea::setScrollbarParams(int h, int scroll_pos, int scroll_line_step)
{
	verticalScrollBar()->setTracking(false);

	// do what cursor movement does (some grey)
	h += height() / 4;
	int scroll_max_ = std::max(0, h - height());

	verticalScrollBar()->setRange(0, scroll_max_);
	verticalScrollBar()->setSliderPosition(scroll_pos);
	verticalScrollBar()->setSingleStep(scroll_line_step);

	verticalScrollBar()->setTracking(true);
}


void GuiWorkArea::adjustViewWithScrollBar(int)
{
	scrollBufferView(verticalScrollBar()->sliderPosition());
}


void GuiWorkArea::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
	/// \todo Ask lyx-devel is this is enough:
	/// if (event->mimeData()->hasFormat("text/plain"))
	///	event->acceptProposedAction();
}


void GuiWorkArea::dropEvent(QDropEvent* event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	lyxerr[Debug::GUI] << "GuiWorkArea::dropEvent: got URIs!" << endl;
	for (int i = 0; i!=files.size(); ++i) {
		string const file = os::internal_path(fromqstr(files.at(i).toLocalFile()));
		if (!file.empty())
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void GuiWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (dc_event_.active && dc_event_ == *e) {
		dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE,
			dc_event_.x, dc_event_.y,
			q_button_state(dc_event_.state));
		dispatch(cmd);
		return;
	}

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
			      q_button_state(e->button()));
	dispatch(cmd);
}


void GuiWorkArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (synthetic_mouse_event_.timeout.running())
		synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			      q_button_state(e->button()));
	dispatch(cmd);
}


void GuiWorkArea::mouseMoveEvent(QMouseEvent * e)
{
	FuncRequest cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
			      q_motion_state(e->button()));

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
		else {
			synthetic_mouse_event_.restart_timeout = true;
			synthetic_mouse_event_.timeout.start();
			// Fall through to handle this event...
		}

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
	if (e->x() != synthetic_mouse_event_.x_old ||
	    e->y() != synthetic_mouse_event_.y_old ||
	    scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.x_old = e->x();
		synthetic_mouse_event_.y_old = e->y();
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		dispatch(cmd);
	}
}


void GuiWorkArea::wheelEvent(QWheelEvent * e)
{
	// Wheel rotation by one notch results in a delta() of 120 (see
	// documentation of QWheelEvent)
	int const lines = qApp->wheelScrollLines() * e->delta() / 120;
	verticalScrollBar()->setValue(verticalScrollBar()->value() -
			lines *  verticalScrollBar()->singleStep());
	adjustViewWithScrollBar();
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


void GuiWorkArea::keyPressEvent(QKeyEvent * e)
{
	lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
		<< " count=" << e->count()
		<< " text=" << fromqstr(e->text())
		<< " isAutoRepeat=" << e->isAutoRepeat()
		<< " key=" << e->key()
		<< endl;

	if (USE_EVENT_PRUNING) {
		keyeventQueue_.push(boost::shared_ptr<QKeyEvent>(new QKeyEvent(*e)));
	}
	else {
		boost::shared_ptr<QLyXKeySym> sym(new QLyXKeySym);
		sym->set(e);
		processKeySym(sym, q_key_state(e->modifiers()));
	}
}


// This is used only if USE_EVENT_PRUNING is defined...
void GuiWorkArea::keyeventTimeout()
{
	bool handle_autos = true;

	while (!keyeventQueue_.empty()) {
		boost::shared_ptr<QKeyEvent> ev = keyeventQueue_.front();

		// We never handle more than one auto repeated
		// char in a list of queued up events.
		if (!handle_autos && ev->isAutoRepeat()) {
			keyeventQueue_.pop();
			continue;
		}

		boost::shared_ptr<QLyXKeySym> sym(new QLyXKeySym);
		sym->set(ev.get());

		lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
				   << " count=" << ev->count()
				   << " text=" <<  fromqstr(ev->text())
				   << " isAutoRepeat=" << ev->isAutoRepeat()
				   << " key=" << ev->key()
				   << endl;

		processKeySym(sym, q_key_state(ev->modifiers()));
		keyeventQueue_.pop();

		handle_autos = false;
	}

	// Restart the timer.
	step_timer_.setSingleShot(true);
	step_timer_.start(25);
}


void GuiWorkArea::mouseDoubleClickEvent(QMouseEvent * e)
{
	dc_event_ = double_click(e);

	if (!dc_event_.active)
		return;

	dc_event_.active = false;

	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
		dc_event_.x, dc_event_.y,
		q_button_state(dc_event_.state));
	dispatch(cmd);
}


void GuiWorkArea::resizeEvent(QResizeEvent *)
{
	verticalScrollBar()->setPageStep(viewport()->height());
	paint_device_ = QPixmap(viewport()->width(), viewport()->height());
	resizeBufferView();
}


void GuiWorkArea::update(int x, int y, int w, int h)
{
	viewport()->update(x, y, w, h);
}


void GuiWorkArea::paintEvent(QPaintEvent * e)
{
	/*
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n QWidget width\t" << this->width()
		<< "\n QWidget height\t" << this->height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< "\n pixmap width\t" << pixmap_->width()
		<< "\n pixmap height\t" << pixmap_->height()
		<< "\n QPaintEvent x\t" << e->rect().x()
		<< "\n QPaintEvent y\t" << e->rect().y()
		<< "\n QPaintEvent w\t" << e->rect().width()
		<< "\n QPaintEvent h\t" << e->rect().height()
		<< endl;
	*/

	QPainter q(viewport());
	q.drawPixmap(e->rect(), paint_device_, e->rect());

	if (show_vcursor_)
		q.drawPixmap(cursor_x_, cursor_y_, vcursor_);

	if (show_hcursor_)
		q.drawPixmap(cursor_x_, cursor_y_ + cursor_h_ - 1, hcursor_);
}


QPixmap GuiWorkArea::copyScreen(int x, int y, int w, int h) const
{
	return paint_device_.copy(x, y, w, h);
}


void GuiWorkArea::drawScreen(int x, int y, QPixmap pixmap)
{
	QPainter q(&paint_device_);
	q.drawPixmap(x, y, pixmap);
	update(x, y, pixmap.width(), pixmap.height());
}


void GuiWorkArea::expose(int x, int y, int w, int h)
{
	/*
	if (x == 0 && y == 0 && w == viewport()->width() && h == viewport()->height()) {
		viewport()->repaint(x, y, w, h);
		return;
	}
	*/

	update(x, y, w, h);
}


void GuiWorkArea::showCursor(int x, int y, int h, CursorShape shape)
{
	if (!qApp->focusWidget())
		return;

	show_vcursor_ = true;

	QColor const & required_color = lcolorcache.get(LColor::cursor);

	if (x==cursor_x_ && y==cursor_y_ && h==cursor_h_
		&& cursor_color_ == required_color
		&& cursor_shape_ == shape) {
		show_hcursor_ = lshape_cursor_;
		update(cursor_x_, cursor_y_, cursor_w_, cursor_h_);
		return;
	}

	// Cache the dimensions of the cursor.
	cursor_x_ = x;
	cursor_y_ = y;
	cursor_h_ = h;
	cursor_color_ = required_color;
	cursor_shape_ = shape;

	switch (cursor_shape_) {
	case BAR_SHAPE:
		// FIXME the cursor width shouldn't be hard-coded!
		cursor_w_ = 2;
		lshape_cursor_ = false;
		break;
	case L_SHAPE:
		cursor_w_ = cursor_h_ / 3;
		lshape_cursor_ = true;
		break;
	case REVERSED_L_SHAPE:
		cursor_w_ = cursor_h_ / 3;
		cursor_x_ -= cursor_w_ - 1;
		lshape_cursor_ = true;
		break;
	}

	// We cache two pixmaps:
	// 1 the vertical line of the cursor.
	// 2 the horizontal line of the L-shaped cursor (if necessary).

	// Draw the new (vertical) cursor.
	vcursor_ = QPixmap(cursor_w_, cursor_h_);
	vcursor_.fill(cursor_color_);

	// Draw the new (horizontal) cursor if necessary.
	if (lshape_cursor_) {
		hcursor_ = QPixmap(cursor_w_, 1);
		hcursor_.fill(cursor_color_);
		show_hcursor_ = true;
	}

	update(cursor_x_, cursor_y_, cursor_w_, cursor_h_);
}


void GuiWorkArea::removeCursor()
{
	show_vcursor_ = false;
	show_hcursor_ = false;

	update(cursor_x_, cursor_y_, cursor_w_, cursor_h_);
}


void GuiWorkArea::inputMethodEvent(QInputMethodEvent * e)
{
	QString const & text = e->commitString();
	if (!text.isEmpty()) {

		lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
			<< " preeditString =" << fromqstr(e->preeditString())
			<< " commitString  =" << fromqstr(e->commitString())
			<< endl;

		int key = 0;
		// needed to make math superscript work on some systems
		// ideally, such special coding should not be necessary
		if (text == "^")
			key = Qt::Key_AsciiCircum;
		// FIXME: Needs for investigation, this key is not really used,
		// the ctor below just check if key is different from 0.
		QKeyEvent ev(QEvent::KeyPress, key,
			Qt::NoModifier, text);
		keyPressEvent(&ev);
	}
	e->accept();
}


void GuiWorkArea::focusInEvent(QFocusEvent * ev)
{
	QAbstractScrollArea::focusInEvent(ev);
	lyx_view_.updateToolbars();
}


void GuiWorkArea::focusOutEvent(QFocusEvent * ev)
{
	QAbstractScrollArea::focusOutEvent(ev);
	lyx_view_.updateToolbars();
}

} // namespace frontend
} // namespace lyx

#include "GuiWorkArea_moc.cpp"
