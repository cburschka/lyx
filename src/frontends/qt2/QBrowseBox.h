// -*- C++ -*-
/**
 * \file QBrowseBox.h
 *
 * Original file taken from klyx 0.10 sources:
 * $Id: QBrowseBox.h,v 1.1 2002/10/09 08:59:02 leuven Exp $
 *
 * \author Kalle Dalheimer ?
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QBROWSEBOX_H
#define QBROWSEBOX_H

#include "qgridview.h"

#include <qwidget.h>
#include <qarray.h>


class QString;
class QPainter;
class QPixmap;


class QBrowseBox : public QGridView
{
  Q_OBJECT
public:
  QBrowseBox( int rows , int cols, QWidget* parent = 0 , const char* name=0 , WFlags f =0  );
  ~QBrowseBox();
  
  void insertItem( const QString& text, int x, int y );
  void insertItem( const char* text, int x, int y ) { insertItem( QString( text ), x, y ); }
  void insertItem( QPixmap pixmap, int x, int y );
  void insertItem( QPixmap pixmap);
  void removeItem( int x, int y );
  void clear();

  QString text( int x, int y );
  QPixmap pixmap( int x, int y);

//  int exec( const QPoint& pos );
//  int exec( int x, int y );
//  int exec( const QWidget* trigger );

    signals:
  void selected(int, int);
    
protected:
  virtual void keyPressEvent( QKeyEvent* e );
  virtual void resizeEvent( QResizeEvent* e );
  virtual void mouseReleaseEvent( QMouseEvent* e );
//  virtual void closeEvent( QCloseEvent* e );
  virtual void mouseMoveEvent( QMouseEvent* e );
  virtual void paintCell( QPainter *, int, int );

private:
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    
  int coordsToIndex( int x, int y );

  QString* _texts;
  QPixmap* _pixmaps;
  QPoint _activecell;
    
};


#endif
