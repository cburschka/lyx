/**
 * \file FormSplashBase.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include <qapplication.h>
#include <qtimer.h>
#include <qframe.h>
#include <qpixmap.h>

#include "FormSplashImpl.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h"

FormSplashBase::FormSplashBase( QWidget* parent,  const char* name, WFlags fl, bool modal )
    : QFrame( parent, name, fl, modal)
{
   string banner_file = LibFileSearch("images", "banner", "xpm");
   if (banner_file.empty())
     return;
   
   QPixmap pm(tostr(banner_file).c_str());
   int w = pm.width();
   int h = pm.height();
   
   setBackgroundPixmap(pm);
   setLineWidth(0);
   setGeometry(QApplication::desktop()->width ()/2-w/2,
	       QApplication::desktop()->height()/2-h/2,
	       w, h );
   
   QTimer::singleShot( 5*1000, this, SLOT(hide()) );
}

FormSplashBase::~FormSplashBase()
{
}


void FormSplashBase::mousePressEvent( QMouseEvent * e)
{
   hide();
}


			   
