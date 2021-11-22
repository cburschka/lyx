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
	: QWidget(parent, Qt::Popup), colwidth_(15), rowheight_(15), minrows_(5), mincols_(5)
{
	init();
	setMouseTracking(true);
}


void InsertTableWidget::init()
{
	rows_ = minrows_;
	cols_ = mincols_;
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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	underMouse_ = geometry().contains(event->globalPosition().toPoint());
#else
	underMouse_ = geometry().contains(event->globalPos());
#endif
	if (!underMouse_) {
		bottom_ = 0;
		right_ = 0;
		update();
		return;
	}

	int const r0 = right_;
	int const b0 = bottom_;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	right_ = event->position().x() / colwidth_ + 1;
	bottom_ = event->position().y() / rowheight_ + 1;
#else
	right_ = event->x() / colwidth_ + 1;
	bottom_ = event->y() / rowheight_ + 1;
#endif

	int const newrows = std::max(minrows_, bottom_ + 1);
	if (rows_ != newrows) {
		rows_ = newrows;
		resetGeometry();
	}

	int const newcols = std::max(mincols_, right_ + 1);
	if (cols_ != newcols) {
		cols_ = newcols;
		resetGeometry();
	}

	if (bottom_ != b0 || right_ != r0) {
		update();
		QString const status = QString("%1x%2").arg(bottom_).arg(right_);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		QToolTip::showText(event->globalPosition().toPoint(), status , this);
#else
		QToolTip::showText(event->globalPos(), status , this);
#endif
	}
}


void InsertTableWidget::mouseReleaseEvent(QMouseEvent * /*event*/)
{
	if (underMouse_) {
		QString const qdata = QString("%1 %2").arg(bottom_).arg(right_);
		lyx::dispatch(FuncRequest(LFUN_TABULAR_INSERT, fromqstr(qdata)));
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
	QPalette const palette = this->palette();
	drawGrid(rows_, cols_, palette.base(), palette.text().color());
	if (underMouse_)
		drawGrid(bottom_, right_, palette.highlight(),
				palette.highlightedText().color());
}


void InsertTableWidget::drawGrid(int const rows, int const cols,
	QBrush const fillBrush, QColor lineColor)
{
	QPainter painter(this);
	painter.setPen(lineColor);
	painter.setBrush(fillBrush);

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
