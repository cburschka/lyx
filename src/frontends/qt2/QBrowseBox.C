/**
 * \file QBrowseBox.C
 *
 * Original file taken from klyx 0.10 sources:
 * $Id: QBrowseBox.C,v 1.4 2002/10/17 09:51:18 poenitz Exp $
 *
 * \author Kalle Dalheimer ?
 *
 * Full author contact details are available in file CREDITS
 */


#include <qstring.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qimage.h>

#include <stdio.h>
#include <math.h>

#include "QBrowseBox.h"


QBrowseBox::QBrowseBox(int rows, int cols, QWidget * parent,
		char const * name, WFlags fl)
	: QGridView()
{
	setNumRows(rows);
	setNumCols(cols);
	setCellWidth(width()/cols);
	setCellHeight(height()/rows);

	texts_   = new QString[rows * cols];
	pixmaps_ = new QPixmap[rows * cols];

	activecell_.setX(0);
	activecell_.setY(0);
	updateCell(0, 0);
	setMouseTracking(true);

	if (style().inherits("QWindowsStyle"))
		setFrameStyle(QFrame::WinPanel | QFrame::Raised);
	else
		setFrameStyle(QFrame::Panel | QFrame::Raised);

	setFocusPolicy(QWidget::StrongFocus);
}



QBrowseBox::~QBrowseBox()
{
	delete [] texts_;
	delete [] pixmaps_;
}


int QBrowseBox::coordsToIndex(int row, int col)
{
	if (col < 0 || col > numCols() || row < 0 ||  row > numRows())
		qDebug("coordsToIndex: invalid coords (%d, %d)\n", row, col);
	return row + col * numCols();
}


void QBrowseBox::insertItem(QString const & text, int row, int col)
{
	texts_[coordsToIndex(row, col)] = text;
}


void QBrowseBox::insertItem(char const * text, int x, int y)
{
	insertItem(QString(text), x, y);
}


void QBrowseBox::insertItem(QPixmap pixmap, int row, int col)
{
	pixmaps_[coordsToIndex(row, col)] = pixmap;
}


void QBrowseBox::insertItem( QPixmap pixmap)
{
	int w = pixmap.width() / numCols();
	int h = pixmap.height() / numRows();

	for (int row = 0; row < numRows(); ++row)
		for (int col = 0; col < numCols(); ++col) {
			QPixmap small(w,h);
			bitBlt(&small,0,0,&pixmap,col*w,row*h,w,h,Qt::CopyROP,false);
			insertItem(small, row, col);
		}

	resize(pixmap.width() + (numCols() + 1) * frameWidth(),
	       pixmap.height() + (numRows() + 1) * frameWidth());
}


void QBrowseBox::removeItem(int row, int col)
{
	texts_[coordsToIndex(row, col)] = "";
	pixmaps_[coordsToIndex(row, col)].resize(0, 0);
}


void QBrowseBox::clear()
{
	for (int row = 0; row < numRows(); ++row)
		for (int col = 0; col < numCols(); ++col)
			removeItem(row, col);
}


QString QBrowseBox::text(int row, int col)
{
	if (col < 0 || col >= numCols() || row < 0 || row >= numRows())
		return "";
	return texts_[coordsToIndex(row, col)];
}


QPixmap QBrowseBox::pixmap(int row, int col)
{
	if (col < 0 || col >= numCols() || row < 0 || row >= numRows())
		return QPixmap();
	return pixmaps_[coordsToIndex(row, col)];
}


void QBrowseBox::keyPressEvent(QKeyEvent * e)
{
	switch(e->key()) {
	case Key_Up:
		moveUp();
		break;
	case Key_Down:
		moveDown();
		break;
	case Key_Left:
		moveLeft();
		break;
	case Key_Right:
		moveRight();
		break;
	case Key_Return:
		emit selected(activecell_.x(), activecell_.y());
		break;
	default:
		e->ignore();
	}
}


void QBrowseBox::mouseReleaseEvent(QMouseEvent * e)
{
	qWarning("mouse release");
	emit selected( activecell_.x(), activecell_.y());
}


//void QBrowseBox::closeEvent(QCloseEvent * e)
//{
//	e->accept();
//	qApp->exit_loop();
//}

void QBrowseBox::paintCell(QPainter * painter, int row, int col)
{
	painter->setClipRect(cellGeometry(row,col));//, QPainter::CoordPainter);
	bool ispixmap = false;

	if (!pixmaps_[coordsToIndex(row,col)].isNull()) {
		painter->drawPixmap(0,0,pixmaps_[coordsToIndex(row, col)]);
		ispixmap = true;
	}

	if (activecell_.x() == row && activecell_.y() == col) {
		if (ispixmap)
			qDrawShadeRect(painter, 0, 0, cellWidth(),
			               cellHeight(), colorGroup(), false, 1);
		else
			qDrawShadePanel(painter, 0, 0, cellWidth(),
			                cellHeight(), colorGroup(), false, 1);
	} else {
		qDrawPlainRect(painter, 0, 0, cellWidth(),
		               cellHeight(), colorGroup().background(), 1);
	}

	if (!texts_[coordsToIndex(row, col)].isEmpty()) {
		painter->drawText(0, 0, cellWidth(),
		                  cellHeight(), AlignLeft,
		                  texts_[coordsToIndex(row, col)]);
	}
	painter->setClipping(false);
}


void QBrowseBox::resizeEvent(QResizeEvent * e)
{
	QGridView::resizeEvent(e);
	setCellWidth(contentsRect().width() / numCols());
	setCellHeight(contentsRect().height() / numRows());
}


void QBrowseBox::mouseMoveEvent(QMouseEvent * e)
{
	qWarning("mouseMoveEvent");
	int x = e->pos().x();
	int y = e->pos().y();

	int cellx;
	int celly;

	if (x < 0 || y < 0 || x > width() || y > height()) {
		// outside the box
		cellx = -1;
		celly = -1;
	} else {
#ifdef WITH_WARNINGS
#warning Is the switch of x and y intended here? (Andre)
#endif
		celly = (int)floor( ((double)x) / ((double)cellWidth()) );
		cellx = (int)floor( ((double)y) / ((double)cellHeight()) );
	}

	if (activecell_.x() != cellx || activecell_.y() != celly) {
		qWarning("update");
		// mouse has been moved to another cell
		int oldactivecellx = activecell_.x();
		int oldactivecelly = activecell_.y();
		activecell_.setX(cellx);
		activecell_.setY(celly);
		// remove old highlighting
		updateCell(oldactivecellx, oldactivecelly);
		// set new highlighting
		updateCell(activecell_.x(), activecell_.y());
	}
}


void QBrowseBox::moveLeft()
{
	int const y = activecell_.y();

	if (y>0)
		activecell_.setY(y - 1);

	updateCell(activecell_.x(), y);
	updateCell(activecell_.x(), activecell_.y());
}


void QBrowseBox::moveRight()
{
	int const y = activecell_.y();

	if (y < numCols() - 1)
		activecell_.setY(y+1);

	updateCell(activecell_.x(), y);
	updateCell(activecell_.x(), activecell_.y());
}


void QBrowseBox::moveUp()
{
	int const x = activecell_.x();

	if (x > 0)
		activecell_.setX(x - 1);

	updateCell(x, activecell_.y());
	updateCell(activecell_.x(), activecell_.y());
}


void QBrowseBox::moveDown()
{
	int const x = activecell_.x();

	if (x < numRows() - 1)
		activecell_.setX(x + 1);

	updateCell(x, activecell_.y());
	updateCell(activecell_.x(), activecell_.y());
}

