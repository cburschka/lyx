/**********************************************************************
** $Id: qgridview.h,v 1.1 2002/10/09 08:59:02 leuven Exp $
**
** Definition of QGridView class
**
** Created: 2001.05.23
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

#ifndef QGRIDVIEW_H
#define QGRIDVIEW_H

#include <qscrollview.h>

#ifndef QT_NO_GRIDVIEW

class QGridViewPrivate;

class Q_EXPORT QGridView : public QScrollView
{
    Q_OBJECT
    Q_PROPERTY( int numRows READ numRows WRITE setNumRows )
    Q_PROPERTY( int numCols READ numCols WRITE setNumCols )
    Q_PROPERTY( int cellWidth READ cellWidth WRITE setCellWidth )
    Q_PROPERTY( int cellHeight READ cellHeight WRITE setCellHeight )
public:

    QGridView( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QGridView();

    int numRows() const;
    virtual void setNumRows( int );
    int numCols() const;
    virtual void setNumCols( int );

    int cellWidth() const;
    virtual void setCellWidth( int );
    int cellHeight() const;
    virtual void setCellHeight( int );
    
    QRect cellRect() const;
    QRect cellGeometry( int row, int column );
    QSize gridSize() const;

    int rowAt( int y ) const;
    int columnAt( int x ) const;

    void repaintCell( int row, int column, bool erase=TRUE );
    void updateCell( int row, int column );
    void ensureCellVisible( int row, int column );

protected:
    virtual void paintCell( QPainter *, int row, int col ) = 0;
    virtual void paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch );

    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );

    virtual void dimensionChange( int, int );

private:
    void drawContents( QPainter* );
    void updateGrid();

    int nrows;
    int ncols;
    int cellw;
    int cellh;
    QGridViewPrivate* d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGridView( const QGridView & );
    QGridView &operator=( const QGridView & );
#endif
};

inline int QGridView::cellWidth() const 
{ return cellw; }

inline int QGridView::cellHeight() const 
{ return cellh; }

inline int QGridView::rowAt( int y ) const 
{ return y / cellh; }

inline int QGridView::columnAt( int x ) const 
{ return x / cellw; }

inline int QGridView::numRows() const 
{ return nrows; }

inline int QGridView::numCols() const 
{return ncols; }

inline QRect QGridView::cellRect() const
{ return QRect( 0, 0, cellw, cellh ); }

inline QSize QGridView::gridSize() const 
{ return QSize( ncols * cellw, nrows * cellh ); }



#endif // QT_NO_GRIDVIEW


#endif // QTABLEVIEW_H
