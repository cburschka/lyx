/**
 * \file QBulletsModule.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include "gettext.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include "BulletsModule.h"
#include "ui/BulletsModuleBase.h"
#include "QBrowseBox.h"

BulletsModule::BulletsModule( QWidget* parent,  const char* name, WFlags fl )
	: BulletsModuleBase(parent, name, fl)
{
   standard = new QBrowseBox(6,6);
   standard->insertItem(QPixmap("standard.xpm"));

   maths = new QBrowseBox(6,6);
   maths->insertItem(QPixmap("amssymb.xpm"));

   QPopupMenu * pm = new QPopupMenu();
   
   QPopupMenu * pm1 = new QPopupMenu(pm);
   pm1->insertItem(standard);

   QPopupMenu * pm2 = new QPopupMenu(pm);
   pm2->insertItem(maths);
   
   
   pm->insertItem("Standard",pm1);
   pm->insertItem("Maths",pm2);
   pm->insertItem("Ding 1");
   pm->insertItem("Ding 2");
   pm->insertItem("Ding 3");
   pm->insertItem("Ding 4");
   
   setbullet1TB->setPopup(pm); 

   connect( standard, SIGNAL( selected(int,int) ),
		 this , SLOT( checkThis(int,int) ) );
   
}

BulletsModule::~BulletsModule()
{
   delete standard;
   delete maths;
     
}

void BulletsModule::setLevel1()
{
   qWarning("no setLevel1() yet");
}

void BulletsModule::checkThis(int x, int y)
{
   qWarning("Check, x: %d y: %d ",x,y);
}

