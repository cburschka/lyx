/**
 * \file QBrowseBox.C
 *
 * Original file taken from klyx 0.10 sources:
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <qstring.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qimage.h>

#include "QBrowseBox.h"

#include <cstdio>
#include <cmath>


QBrowseBox::QBrowseBox(int rows, int cols, QWidget* parent, const char * name, WFlags f)
	: QGridView(parent,name,f)
{
	setNumRows(rows);
	setNumCols(cols);

	pixmaps_ = new QPixmap[rows * cols];

	activecell_.setX(-1);
	activecell_.setY(-1);

	if (style().inherits("QWindowsStyle"))
		setFrameStyle(QFrame::WinPanel | QFrame::Raised);
	else
		setFrameStyle(QFrame::Panel | QFrame::Raised);

	setVScrollBarMode(QScrollView::AlwaysOff);
	setHScrollBarMode(QScrollView::AlwaysOff);

	viewport()->setFocusPolicy(QWidget::StrongFocus);
	// setMouseTracking must be called after setFocusPolicy
	viewport()->setMouseTracking(true);
	inloop=false;

}


QBrowseBox::~QBrowseBox()
{
	delete [] pixmaps_;
}


int QBrowseBox::coordsToIndex(int row, int col)
{
	if (col < 0 || col > numCols() || row < 0 ||  row > numRows())
		qDebug("coordsToIndex: invalid coords (%d, %d)\n", row, col);
	return row + col * numCols();
}


void QBrowseBox::insertItem(QPixmap pixmap, int row, int col)
{
	pixmaps_[coordsToIndex(row, col)] = pixmap;
}


void QBrowseBox::insertItem(QPixmap pixmap)
{
	int w = pixmap.width() / numCols();
	int h = pixmap.height() / numRows();

	for (int row = 0; row < numRows(); ++row) {
		for (int col = 0; col < numCols(); ++col) {
			QPixmap small(w,h);
			small.fill(backgroundColor());
			bitBlt(&small, 0, 0, &pixmap, col * w, row * h,
				w, h, Qt::CopyROP, false);
			insertItem(small, row, col);
		}
	}

	setCellWidth(pixmap.width() / numCols());
	setCellHeight(pixmap.height() / numRows());
	setMinimumWidth(pixmap.width() + (numCols() + 1) * 1);
	setMinimumHeight(pixmap.height() + (numRows() + 1) * 1);
	resize(minimumSize());
}


QPixmap QBrowseBox::pixmap(int row, int col)
{
	if (col < 0 || col >= numCols() || row < 0 || row >= numRows())
		return QPixmap();
	return pixmaps_[coordsToIndex(row, col)];
}


int QBrowseBox::exec(const QPoint & pos)
{
	return exec(pos.x(),pos.y());
}


int QBrowseBox::exec(const QWidget * trigger)
{
	QPoint globalpos = trigger->parentWidget()->mapToGlobal( trigger->pos());
	// is there enough space to put the box below the trigger?
	if ( globalpos.y() + trigger->height()+height()+1<
	     QApplication::desktop()->height()) {
		// is there enough space to set the box left-justified with the trigger
		if (globalpos.x()+width()<QApplication::desktop()->width())
			return exec(globalpos.x(),
				    globalpos.y()+trigger->height()+1);
		else
			return exec(globalpos.x()-width()-1,
				    globalpos.y()+trigger->height()+1);
	} else {
		if (globalpos.x()+width()<QApplication::desktop()->width())
			return exec(globalpos.x(),
				    globalpos.y()-height()-1);
		else
			return exec(globalpos.x()-width()-1,
				    globalpos.y()-height()-1);
	}
}


int QBrowseBox::exec(int x,int y)
{
	firstrelease_ = true;
	move(x,y);
	show();
	repaint();
	qApp->enter_loop();
	inloop = true;

	if (activecell_.x()!=-1 && activecell_.y()!=-1 )
		return coordsToIndex( activecell_.x(),activecell_.y());
	else
		return -1;
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
		if ( isVisible() && parentWidget() &&
		     parentWidget()->inherits("QPopupMenu") )
			parentWidget()->close();
		else
			close();
		break;
	case Key_Escape:
		if (inloop)
			qApp->exit_loop();
		if ( isVisible() && parentWidget() &&
		     parentWidget()->inherits("QPopupMenu") )
			parentWidget()->close();
	default:
		e->ignore();
	}
}


void QBrowseBox::contentsMouseReleaseEvent(QMouseEvent *)
{

	if (firstrelease_)
		firstrelease_ = false;
	else {
		emit selected( activecell_.x(), activecell_.y());
		if ( isVisible() && parentWidget() &&
		     parentWidget()->inherits("QPopupMenu") )
			parentWidget()->close();
	}
}


void QBrowseBox::closeEvent(QCloseEvent * e)
{
	e->accept();
	qApp->exit_loop();
}


void QBrowseBox::paintCell(QPainter * painter, int row, int col)
{
	painter->setClipRect(cellGeometry(row, col));

	int const index = coordsToIndex(row, col);

	painter->drawPixmap(0, 0, pixmaps_[index]);

	if (activecell_.x() == row && activecell_.y() == col) {
		qDrawShadeRect(painter, 0, 0, cellWidth(),
			       cellHeight(), colorGroup(), false, 1);
	} else {
		qDrawPlainRect(painter, 0, 0, cellWidth(),
			       cellHeight(), colorGroup().background(), 1);
	}

	painter->setClipping(false);
}


void QBrowseBox::contentsMouseMoveEvent(QMouseEvent * e)
{
	int x = e->pos().x();
	int y = e->pos().y();

	int cellx;
	int celly;

	if (x < 0 || y < 0 || x > width() || y > height()) {
		// outside the box
		cellx = -1;
		celly = -1;
	} else {
		celly = (int)floor( ((double)x) / ((double)cellWidth()) );
		cellx = (int)floor( ((double)y) / ((double)cellHeight()) );
	}

	if (activecell_.x() != cellx || activecell_.y() != celly) {
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
	else if (parentWidget())
		QWidget::focusNextPrevChild(false);

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
	else if (parentWidget())
		QWidget::focusNextPrevChild(false);

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
