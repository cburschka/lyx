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

#include <qevent.h>
#include <qpainter.h>
 
 
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
	wa_->workAreaButtonPress(e->x(), e->y(), q_button_state(e->button()));
}


void QContentPane::mouseReleaseEvent(QMouseEvent * e)
{
	wa_->workAreaButtonRelease(e->x(), e->y(), q_button_state(e->button()));
}

 
void QContentPane::mouseMoveEvent(QMouseEvent * e)
{
	wa_->workAreaMotionNotify(e->x(), e->y(), q_motion_state(e->state()));
}


void QContentPane::keyPressEvent(QKeyEvent * e)
{
	char const * tmp = e->text().latin1();
	string const text = tmp ? tmp : "";
	lyxerr[Debug::GUI] << "key text " << text << endl;
	QLyXKeySym * sym = new QLyXKeySym();
	sym->set(e->key(), e->text());
	wa_->workAreaKeyPress(LyXKeySymPtr(sym), q_key_state(e->state()));
}

 
void QContentPane::mouseDoubleClickEvent(QMouseEvent * e)
{
	wa_->workAreaDoubleClick(e->x(), e->y(), q_button_state(e->state()));
	// FIXME: triple click 
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
