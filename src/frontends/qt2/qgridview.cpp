/****************************************************************************
** $Id: qgridview.cpp,v 1.1 2002/10/09 08:59:02 leuven Exp $
**
** Implementation of QGridView class
**
** Created: 010523
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#include "qgridview.h"

#ifndef QT_NO_GRIDVIEW

#include <qpainter.h>

/*!
  \class QGridView qgridview.h
  \brief The QGridView class provides an abstract base for fixed-size
  grids.

  \ingroup abstractwidgets

  A grid view consists of a number of abstract cells organized in rows
  and columns. The cells have a fixed size and are identified with a
  row index and a column index. The top-left cell is in row 0, column
  0. The bottom-right cell is in row numRows()-1, column numCols()-1.

  You can define \l numRows, \l numCols, \l cellWidth and
  \l cellHeight. Reimplement the pure virtual function paintCell() to
  draw the content of a cell.

  With ensureCellVisible(), you can ensure a certain cell is
  visible. With rowAt() and columnAt() you can find a cell based on
  the given x- and y-coordinates.

  If you need to monitor changes to the grid's dimensions (i.e. when
  numRows or numCols is changed), reimplement the dimensionChange()
  change handler.

  Note: the row, column indices are always given in the order, row
  (vertical offset) then column (horizontal offset). This order is the
  opposite of all pixel operations, which are given in the order x
  (horizontal offset), y (vertical offset).

  QGridView is a very simple abstract class based on QScrollView. It
  is designed to simplify the task of drawing many cells of the same
  size in a potentially scrollable canvas. If you need rows and
  columns in different sizes, use a QTable instead. If you need a
  simple list of items, use a QListBox. If you need to present
  hierachical data use a QListView, and if you need random objects at
  random positions, consider using either a QIconView or a QCanvas.

*/


/*!
  Constructs a grid view.

  The \a parent, \a name and widget flag, \a f, arguments are passed to the
  QScrollView constructor.
*/
QGridView::QGridView( QWidget *parent, const char *name, WFlags f )
    :QScrollView( parent, name ,f ),nrows(5),ncols(5),cellw(12),cellh(12)
{
    viewport()->setBackgroundMode( PaletteBase );
    setBackgroundMode( PaletteBackground );
    viewport()->setFocusProxy( this );
}

/*!
  Destroys the grid view.
*/
QGridView::~QGridView()
{
}

void QGridView::updateGrid()
{
    resizeContents( ncols * cellw, nrows * cellh );
}

/*! \property QGridView::numRows
  \brief The number of rows in the grid

  \sa numCols
*/
void QGridView::setNumRows( int numRows )
{
    int oldnrows = nrows;
    nrows = numRows;
    dimensionChange( oldnrows, ncols );
    updateGrid();
}

/*! \property QGridView::numCols
  \brief The number of columns in the grid

  \sa numRows
*/
void QGridView::setNumCols( int numCols )
{
    int oldncols = ncols;
    ncols = numCols;
    dimensionChange( nrows, oldncols );
    updateGrid();
}

/*! \property QGridView::cellWidth
  \brief The width of a grid column

  All columns in a grid view have the same width.

  \sa cellHeight
*/
void QGridView::setCellWidth( int cellWidth )
{
    cellw = cellWidth;
    updateGrid();
    updateContents( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
    
}

/*! \property QGridView::cellHeight
  \brief The height of a grid row

  All rows in a grid view have the same height.

  \sa cellWidth
*/
void QGridView::setCellHeight( int cellHeight )
{
    cellh = cellHeight;
    updateGrid();
    updateContents( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
    
}

/*!
  Returns the geometry of cell (\a row, \a column) in the content
  coordinate system.

  \sa cellRect()
 */
QRect QGridView::cellGeometry( int row, int column )
{
    QRect r;
    if ( row >= 0 && row < nrows && column >= 0 && column < ncols )
	r.setRect( cellw * column, cellh * row, cellw, cellh );
    return r;
}

/*!  Repaints cell (\a row, \a column).

  If \a erase is TRUE, Qt erases the area of the cell before the
  paintCell() call; otherwise no erasing takes place.

  \sa QWidget::repaint()
 */
void QGridView::repaintCell( int row, int column, bool erase )
{
    repaintContents( cellGeometry( row, column ), erase );
}

/*!  Updates cell (\a row, \a column).

  \sa QWidget::update()
 */
void QGridView::updateCell( int row, int column )
{
    updateContents( cellGeometry( row, column ) );
}

/*!
  Ensure cell (\a row, \a column) is visible, scrolling the grid view
  if necessary.
 */
void QGridView::ensureCellVisible( int row, int column )
{
    QRect r = cellGeometry( row, column );
    ensureVisible( r.x(), r.y(), r.width(), r.height() );
}

/*! This function fills the \a cw pixels wide and \a ch pixels high
  rectangle starting at position (\a cx, \a cy) with the
  background color using the painter \a p.

  paintEmptyArea() is invoked by drawContents() to erase
  or fill unused areas.
*/

void QGridView::paintEmptyArea( QPainter *p, int cx ,int cy, int cw, int ch)
{
    if ( gridSize().width() >= contentsWidth() && gridSize().height() >= contentsHeight() )
	return;
    // Region of the rect we should draw
    contentsToViewport( cx, cy, cx, cy );
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( QRect( contentsToViewport( QPoint( 0, 0 ) ), gridSize() ) );

    // And draw the rectangles (transformed as needed)
    QArray<QRect> r = reg.rects();
    const QBrush &brush = colorGroup().brush(QColorGroup::Background);
    for ( int i = 0; i < (int)r.count(); ++i)
	p->fillRect( r[ i ], brush );
}

/*!\reimp
 */
void QGridView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    int colfirst = columnAt( cx );
    int collast = columnAt( cx + cw );
    int rowfirst = rowAt( cy );
    int rowlast = rowAt( cy + ch );

    if ( rowfirst == -1 || colfirst == -1 ) {
	paintEmptyArea( p, cx, cy, cw, ch );
	return;
    }

    if ( collast < 0 || collast >= ncols )
	collast = ncols-1;
    if ( rowlast < 0 || rowlast >= nrows )
	rowlast = nrows-1;

    // Go through the rows
    for ( int r = rowfirst; r <= rowlast; ++r ) {
	// get row position and height
	int rowp = r * cellh;

	// Go through the columns in the row r
	// if we know from where to where, go through [colfirst, collast],
	// else go through all of them
	for ( int c = colfirst; c <= collast; ++c ) {
	    // get position and width of column c
	    int colp = c * cellw;
	    // Translate painter and draw the cell
	    p->translate( colp, rowp );
	    paintCell( p, r, c );
	    p->translate( -colp, -rowp );
	}
    }

    // Paint empty rects
    paintEmptyArea( p, cx, cy, cw, ch );
}

/*!
  \reimp

  (Implemented to get rid of a compiler warning.)
*/
void QGridView::drawContents( QPainter * )
{
}

/*! \fn void QGridView::dimensionChange( int oldNumRows, int oldNumCols )

  This change handler is called whenever any of the grid's dimensions
  changes. \a oldNumRows and \a oldNumCols contain the old dimensions,
  numRows() and numCols() contain the new dimensions.
 */
void QGridView::dimensionChange( int, int ) {}



/*! \fn int QGridView::rowAt( int y ) const

  Returns the number of the row at position \a y. \a y must be given in
  content coordinates.

  \sa columnAt()
 */

/*! \fn int QGridView::columnAt( int x ) const

  Returns the number of the column at position \a x. \a x must be
  given in content coordinates.

  \sa rowAt()
 */

/*!
  \fn void QGridView::paintCell( QPainter *p, int row, int col )

  This pure virtual function is called to paint the single cell at
  (\a row, \a col) using painter \a p. The painter must be open when
  paintCell() is called and must remain open.

  The coordinate system is \link QPainter::translate() translated \endlink
  so that the origin is at the top-left corner of the cell to be
  painted, i.e. \e cell coordinates.  Do not scale or shear the coordinate
  system (or if you do, restore the transformation matrix before you
  return).

  The painter is not clipped by default in order to get maximum
  efficiency. If you want clipping, use

  \code
    p->setClipRect( cellRect(), QPainter::CoordPainter );
    //... your drawing code
    p->setClipping( FALSE );

 \endcode

*/

/*! \fn  QRect QGridView::cellRect() const

  Returns the geometry of a cell in a cell's coordinate system. This
  is a convenience function useful in paintCell(). It is equivalent to
  QRect( 0, 0, cellWidth(), cellHeight() ).

  \sa cellGeometry()

 */

/*!\fn  QSize QGridView::gridSize() const

  Returns the size of the grid in pixels.

 */

#endif // QT_NO_GRIDVIEW
