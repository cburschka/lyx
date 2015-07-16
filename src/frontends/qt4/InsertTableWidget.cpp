/**
 * \file InsertTableWidget.cpp
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsertTableWidget.h"

#include "GuiView.h"
#include "qt_helpers.h"

// DispatchResult.h is needed by the windows compiler because lyx::dispatch
// returns a DispatchResult const reference. Gcc does not complain. Weird...
#include "DispatchResult.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "LyX.h"

#include <QMouseEvent>
#include <QPainter>
#include <QString>
#include <QToolTip>


namespace lyx {
namespace frontend {

InsertTableWidget::InsertTableWidget(QWidget * parent)
	: QWidget(parent, Qt::Popup), colwidth_(20), rowheight_(12)
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
	// emit signal
	visible(true);
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
	if (!underMouse_) {
		bottom_ = 0;
		right_ = 0;
		update();
		return;
	}

	int const r0 = right_;
	int const b0 = bottom_;
	right_ = event->x() / colwidth_ + 1;
	bottom_ = event->y() / rowheight_ + 1;

	if (bottom_ == rows_) {
		++rows_;
		resetGeometry();
	}

	if (right_ == cols_) {
		++cols_;
		resetGeometry();
	}

	if (bottom_ != b0 || right_ != r0) {
		update();
		QString const status = QString("%1x%2").arg(bottom_).arg(right_);
		QToolTip::showText(event->globalPos(), status , this);
	}
}


void InsertTableWidget::mouseReleaseEvent(QMouseEvent * /*event*/)
{
	if (underMouse_) {
		QString const data = QString("%1 %2").arg(bottom_).arg(right_);
		lyx::dispatch(FuncRequest(LFUN_TABULAR_INSERT, fromqstr(data)));
	}
	// emit signal
	visible(false);
	close();
}


void InsertTableWidget::mousePressEvent(QMouseEvent * /*event*/)
{
	// swallow this one
}


void InsertTableWidget::paintEvent(QPaintEvent * /*event*/)
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
	bool status = getStatus(FuncRequest(LFUN_TABULAR_INSERT)).enabled();
	parentWidget()->setEnabled(status);
}


} // namespace frontend
} // namespace lyx

#include "moc_InsertTableWidget.cpp"
