/**
 * \file QContentPane.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "QWorkArea.h"

#include "QContentPane.h"
#include "QLyXKeySym.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>

#include <boost/bind.hpp>

namespace {

/// return the LyX key state from Qt's
key_modifier::state q_key_state(Qt::ButtonState state)
{
	key_modifier::state k = key_modifier::none;
	if (state & Qt::ControlButton)
		k |= key_modifier::ctrl;
	if (state & Qt::ShiftButton)
		k |= key_modifier::shift;
	if (state & Qt::AltButton)
		k |= key_modifier::alt;
	return k;
}


/// return the LyX mouse button state from Qt's
mouse_button::state q_button_state(Qt::ButtonState button)
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
mouse_button::state q_motion_state(Qt::ButtonState state)
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


SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true),
	  x_old(-1), y_old(-1), scrollbar_value_old(-1.0)
{}


QContentPane::QContentPane(QWorkArea * parent)
	: QWidget(parent, "content_pane", WRepaintNoErase),
	  track_scrollbar_(true), wa_(parent)
{
	synthetic_mouse_event_.timeout.timeout.connect(
		boost::bind(&QContentPane::generateSyntheticMouseEvent,
			    this));

	setFocusPolicy(QWidget::WheelFocus);
	setFocus();
	setCursor(ibeamCursor);

	// stupid moc strikes again
	connect(wa_->scrollbar_, SIGNAL(valueChanged(int)),
		this, SLOT(scrollBarChanged(int)));
}


void QContentPane::generateSyntheticMouseEvent()
{
	// Set things off to generate the _next_ 'pseudo' event.
	if (synthetic_mouse_event_.restart_timeout)
		synthetic_mouse_event_.timeout.start();

	// Has anything changed on-screen since the last timeout signal
	// was received?
	double const scrollbar_value = wa_->scrollbar_->value();
	if (scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		wa_->dispatch(synthetic_mouse_event_.cmd);
	}
}


void QContentPane::scrollBarChanged(int val)
{
	if (track_scrollbar_)
		wa_->scrollDocView(val);
}


void QContentPane::mousePressEvent(QMouseEvent * e)
{
	if (dc_event_.active && dc_event_ == *e) {
		dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE,
			dc_event_.x, dc_event_.y,
			q_button_state(dc_event_.state));
		wa_->dispatch(cmd);
		return;
	}

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
			      q_button_state(e->button()));
	wa_->dispatch(cmd);
}


void QContentPane::mouseReleaseEvent(QMouseEvent * e)
{
	if (synthetic_mouse_event_.timeout.running())
		synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			      q_button_state(e->button()));
	wa_->dispatch(cmd);
}


void QContentPane::mouseMoveEvent(QMouseEvent * e)
{
	FuncRequest const cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
			      q_motion_state(e->state()));

	// If we're above or below the work area...
	if (e->y() <= 0 || e->y() >= height()) {
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
	double const scrollbar_value = wa_->scrollbar_->value();
	if (e->x() != synthetic_mouse_event_.x_old ||
	    e->y() != synthetic_mouse_event_.y_old ||
	    scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.x_old = e->x();
		synthetic_mouse_event_.y_old = e->y();
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		wa_->dispatch(cmd);
	}
}


void QContentPane::wheelEvent(QWheelEvent * e)
{
	wa_->scrollbar_->setValue(wa_->scrollbar_->value() - e->delta());
}


void QContentPane::keyPressEvent(QKeyEvent * e)
{
	typedef boost::shared_ptr<LyXKeySym> LyXKeySymPtr;

	QLyXKeySym * sym = new QLyXKeySym;
	sym->set(e);
	wa_->workAreaKeyPress(LyXKeySymPtr(sym), q_key_state(e->state()));
}


void QContentPane::doubleClickTimeout()
{
	if (!dc_event_.active)
		return;

	dc_event_.active = false;

	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
		dc_event_.x, dc_event_.y,
		q_button_state(dc_event_.state));
	wa_->dispatch(cmd);
}


void QContentPane::mouseDoubleClickEvent(QMouseEvent * e)
{
	dc_event_ = double_click(e);

	// doubleClickInterval() is just too long.
	QTimer::singleShot(int(QApplication::doubleClickInterval() / 1.5),
		this, SLOT(doubleClickTimeout()));
}


void QContentPane::resizeEvent(QResizeEvent *)
{
	if (!pixmap_.get()) {
		pixmap_.reset(new QPixmap(width(), height()));
	}

	pixmap_->resize(width(), height());
	wa_->workAreaResize();
}


void QContentPane::paintEvent(QPaintEvent * e)
{
	if (!pixmap_.get()) {
		pixmap_.reset(new QPixmap(width(), height()));
		wa_->workAreaResize();
		return;
	}

	QRect r(e->rect());

	QPainter q(this);
	q.drawPixmap(QPoint(r.x(), r.y()),
		*pixmap_.get(), r);
}


void QContentPane::trackScrollbar(bool track_on)
{
	track_scrollbar_ = track_on;
}
