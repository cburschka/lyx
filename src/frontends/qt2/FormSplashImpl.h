/**
 * \file FormSplashBase.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef FORMSPLASHBASE_H
#define FORMSPLASHBASE_H

class FormSplashBase : public QFrame
{
 public:
   
   FormSplashBase(QWidget* parent,  const char* name,
		  WFlags fl = QWidget::WStyle_NoBorder | QWidget::WStyle_Customize, 
		  bool modal = false );
   
   ~FormSplashBase();
   
 protected:
   
   virtual void mousePressEvent( QMouseEvent * );
};

#endif // FORMSPLASHBASE_H
