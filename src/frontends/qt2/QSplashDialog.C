/**
 * \file QSplashDialog.C
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

#include "version.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "QSplashDialog.h" 

QSplashDialog::QSplashDialog(QWidget * parent, const char * name, WFlags fl, bool modal)
	 : QFrame(parent, name, fl, modal)
{
	string const banner_file = LibFileSearch("images", "banner", "xpm");
	if (banner_file.empty())
		return;
	
	QPixmap const pm(tostr(banner_file).c_str());
	int const w = pm.width();
	int const h = pm.height();
	
	setBackgroundPixmap(pm);
	setLineWidth(0);
	setGeometry(QApplication::desktop()->width()/2 - w/2,
			 QApplication::desktop()->height()/2 - h/2,
			 w, h);
	
	QTimer::singleShot(5*1000, this, SLOT(hide()));
}

 
QSplashDialog::~QSplashDialog()
{
}


void QSplashDialog::mousePressEvent( QMouseEvent * )
{
	hide();
}
