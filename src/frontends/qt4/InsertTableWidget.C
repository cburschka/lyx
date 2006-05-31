/**
 * \file InsertTableWidget.C
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"	// needed for lyxfunc
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "funcrequest.h"
#include "LyXView.h"
#include "debug.h"

#include "qt_helpers.h"

#include "InsertTableWidget.h"
#include <QMouseEvent>
#include <QString>
#include <QToolTip>
#include <QPainter>
#include <QCoreApplication>


namespace lyx {
namespace frontend {

InsertTableWidget::InsertTableWidget(LyXView & lyxView, FuncRequest const & func, QWidget * parent)
	: QWidget(parent, Qt::Popup), colwidth_(20), rowheight_(12), lyxView_(lyxView), func_(func)
{
	init();
	setMouseTracking(true);
}


void InsertTableWidget::init()
{
	rows_ = 5;
	cols_ = 5;
	bottom_ = 0;
	right_ = 0;
	underMouse_ = false;
}


void InsertTableWidget::show(bool show)
{
	if (!show)
		return;

	init();
	resetGeometry();
	setVisible(true);
	emit visible(true);
}


void InsertTableWidget::resetGeometry()
{
	QPoint p = parentWidget()->mapToGlobal(parentWidget()->geometry().bottomLeft());
	setGeometry(p.x() - parentWidget()->pos().x(), 
				p.y() - parentWidget()->pos().y(),
				cols_ * colwidth_ + 1, rows_ * rowheight_ + 1);
}


void InsertTableWidget::mouseMoveEvent(QMouseEvent * event)
{
	// do this ourselves because when the mouse leaves the app
	// we get an enter event (ie underMouse() is true)!!
	underMouse_ = geometry().contains(event->globalPos());
	if (!underMouse_)
		return;

	int const r0 = right_;
	int const b0 = bottom_;
	right_ = event->x()/colwidth_ + 1;
	bottom_ = event->y()/rowheight_ + 1;

	if (bottom_ == rows_) {
		++rows_;
		resetGeometry();
	}

	if (right_ == cols_) {
		++cols_;
		resetGeometry();
	}

	if (bottom_ != b0 || right_ != r0)
		update();

	QString status = QString("%1x%2").arg(bottom_).arg(right_);
	QToolTip::showText(event->globalPos(), status , this); 
}


bool InsertTableWidget::event(QEvent * event)
{
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent * me = dynamic_cast<QMouseEvent *>(event);
		mouseMoveEvent(me);
		return true;
	} else if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent * me = dynamic_cast<QMouseEvent *>(event);
		mouseReleaseEvent(me);
		return true;
	} else if (event->type() == QEvent::MouseButtonPress) {
		// swallow this one...
		return true;
	} else if (event->type() == QEvent::Leave) {
		bottom_ = 0;
		right_ = 0;
		update();
		return true;
	}
	return QWidget::event(event);
}


void InsertTableWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (underMouse_) {
		QString const data = QString("%1 %2").arg(bottom_).arg(right_);
		lyxView_.getLyXFunc().dispatch(FuncRequest(LFUN_TABULAR_INSERT, fromqstr(data)));
	}
	emit visible(false);
	close();
}


void InsertTableWidget::paintEvent(QPaintEvent * event)
{
	drawGrid(rows_, cols_, Qt::white);
	if (underMouse_)
		drawGrid(bottom_, right_, Qt::darkBlue);
}


void InsertTableWidget::drawGrid(int const rows, int const cols, Qt::GlobalColor const color)
{
	QPainter painter(this);
	painter.setPen(Qt::darkGray);
	painter.setBrush(color);

	for (int r = 0 ; r < rows ; ++r ) {
		for (int c = 0 ; c < cols ; ++c ) {
			QRect rectangle(c * colwidth_, r * rowheight_, colwidth_, rowheight_);
			painter.drawRect(rectangle);
		}
	}
}


void InsertTableWidget::updateParent()
{
	FuncStatus const status = lyxView_.getLyXFunc().getStatus(func_);
	parentWidget()->setEnabled(status.enabled());
}


} // namespace frontend
} // namespace lyx

#include "InsertTableWidget_moc.cpp"
