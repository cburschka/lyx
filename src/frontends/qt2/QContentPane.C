/**
 * \file QContentPane.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"

#include "QWorkArea.h"
#include "QLyXKeySym.h"
#include "funcrequest.h"

#include <qevent.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qapplication.h>
 
using std::endl;

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
 

QContentPane::QContentPane(QWorkArea * parent)
	: QWidget(parent, "content_pane", WRepaintNoErase), 
	wa_(parent)
{
	setFocusPolicy(QWidget::WheelFocus);
	setFocus();

	// stupid moc strikes again
	connect(wa_->scrollbar_, SIGNAL(valueChanged(int)), 
		this, SLOT(scrollBarChanged(int)));
 
}


void QContentPane::scrollBarChanged(int val)
{
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
 
	FuncRequest cmd
		(LFUN_MOUSE_PRESS, e->x(), e->y(), q_button_state(e->button()));
	wa_->dispatch(cmd);
}


void QContentPane::mouseReleaseEvent(QMouseEvent * e)
{
	FuncRequest cmd
		(LFUN_MOUSE_RELEASE, e->x(), e->y(), q_button_state(e->button()));
	wa_->dispatch(cmd);
}

 
void QContentPane::mouseMoveEvent(QMouseEvent * e)
{
	FuncRequest cmd
		(LFUN_MOUSE_MOTION, e->x(), e->y(), q_motion_state(e->state()));
	wa_->dispatch(cmd);
}


void QContentPane::keyPressEvent(QKeyEvent * e)
{
	lyxerr[Debug::KEY] << "Press key " << e->key()
		<< " text \"" << (e->text().isEmpty() ? "none" : e->text().latin1())
		<< "\", ascii \"" << e->ascii() << "\"" << endl;
	QLyXKeySym * sym = new QLyXKeySym();
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
 
	lyxerr[Debug::GUI] << "repainting " << r.x() 
		<< "," << r.y() << " " << r.width() 
		<< "," << r.height() << endl;
	QPainter q(this);
	q.drawPixmap(QPoint(r.x(), r.y()),
		*pixmap_.get(), r); 
}
