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
#include "support/filetools.h"


BulletsModule::BulletsModule(QWidget * parent,  char const * name, WFlags fl)
	: BulletsModuleBase(parent, name, fl)
{

	QPopupMenu * pm = new QPopupMenu(this);
	
	QPopupMenu * pm1 = new QPopupMenu(pm);
	QPopupMenu * pm2 = new QPopupMenu(pm);
	QPopupMenu * pm3 = new QPopupMenu(pm);
	QPopupMenu * pm4 = new QPopupMenu(pm);
	QPopupMenu * pm5 = new QPopupMenu(pm);
	QPopupMenu * pm6 = new QPopupMenu(pm);

	standard = new QBrowseBox(6, 6, pm1);
	maths = new QBrowseBox(6, 6, pm2);
	ding1 = new QBrowseBox(6, 6, pm3);
	ding2 = new QBrowseBox(6, 6, pm4);
	ding3 = new QBrowseBox(6, 6, pm5);
	ding4 = new QBrowseBox(6, 6, pm6);

	pm1->insertItem(standard);
	pm2->insertItem(maths);
	pm3->insertItem(ding1);
	pm4->insertItem(ding2);
	pm5->insertItem(ding3);
	pm6->insertItem(ding4);

	pm->insertItem("Standard", pm1);
	pm->insertItem("Maths", pm2);
	pm->insertItem("Ding 1",pm3);
	pm->insertItem("Ding 2",pm4);
	pm->insertItem("Ding 3",pm5);
	pm->insertItem("Ding 4",pm6);

	setbullet1TB->setPopup(pm);
	setbullet2TB->setPopup(pm);
	setbullet3TB->setPopup(pm);
	setbullet4TB->setPopup(pm);

	// insert pixmaps
	string bmfile;
	bmfile = LibFileSearch("images", "standard", "xpm");
	standard->insertItem(QPixmap(bmfile.c_str()));
	
	bmfile = LibFileSearch("images", "amssymb", "xpm");
	maths->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss1", "xpm");
	ding1->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss2", "xpm");
	ding2->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss3", "xpm");
	ding3->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss4", "xpm");
	ding4->insertItem(QPixmap(bmfile.c_str()));

	connect(standard, SIGNAL(selected(int, int)),
		this, SLOT(checkThis(int, int)));

}


BulletsModule::~BulletsModule()
{
}


void BulletsModule::setLevel1()
{
	qWarning("no setLevel1() yet");
}


void BulletsModule::checkThis(int x, int y)
{
	qWarning("Check, x: %d y: %d ", x, y);
}
