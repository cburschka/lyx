/**
 * \file BulletsModule.C
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
#include <qlineedit.h>
#include <qcombobox.h>
#include "BulletsModule.h"
#include "ui/BulletsModuleBase.h"
#include "QBrowseBox.h"
#include "support/filetools.h"
#include "bulletstrings.h"

BulletsModule::BulletsModule(QWidget * parent,  char const * name, WFlags fl)
	: BulletsModuleBase(parent, name, fl), le_(bullet1LE)
{
	QPopupMenu * pm = new QPopupMenu(this);
	
	QPopupMenu * pm1 = new QPopupMenu(pm);
	QPopupMenu * pm2 = new QPopupMenu(pm);
	QPopupMenu * pm3 = new QPopupMenu(pm);
	QPopupMenu * pm4 = new QPopupMenu(pm);
	QPopupMenu * pm5 = new QPopupMenu(pm);
	QPopupMenu * pm6 = new QPopupMenu(pm);

	standard_ = new QBrowseBox(6, 6, pm1);
	maths_ = new QBrowseBox(6, 6, pm2);
	ding1_ = new QBrowseBox(6, 6, pm3);
	ding2_ = new QBrowseBox(6, 6, pm4);
	ding3_ = new QBrowseBox(6, 6, pm5);
	ding4_ = new QBrowseBox(6, 6, pm6);

	pm1->insertItem(standard_);
	pm2->insertItem(maths_);
	pm3->insertItem(ding1_);
	pm4->insertItem(ding2_);
	pm5->insertItem(ding3_);
	pm6->insertItem(ding4_);

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
	standard_->insertItem(QPixmap(bmfile.c_str()));
	
	bmfile = LibFileSearch("images", "amssymb", "xpm");
	maths_->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss1", "xpm");
	ding1_->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss2", "xpm");
	ding2_->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss3", "xpm");
	ding3_->insertItem(QPixmap(bmfile.c_str()));

	bmfile = LibFileSearch("images", "psnfss4", "xpm");
	ding4_->insertItem(QPixmap(bmfile.c_str()));

	connect(standard_, SIGNAL(selected(int, int)),
		this, SLOT(standard(int, int)));

	connect(maths_, SIGNAL(selected(int, int)),
		this, SLOT(maths(int, int)));
	
	connect(ding1_, SIGNAL(selected(int, int)),
		this, SLOT(ding1(int, int)));
	
	connect(ding2_, SIGNAL(selected(int, int)),
		this, SLOT(ding2(int, int)));
	
	connect(ding3_, SIGNAL(selected(int, int)),
		this, SLOT(ding3(int, int)));
	
	connect(ding4_, SIGNAL(selected(int, int)),
		this, SLOT(ding4(int, int)));
	
	connect(setbullet1TB, SIGNAL(pressed()),
		this, SLOT(setlevel1()));
	
	connect(setbullet2TB, SIGNAL(pressed()),
		this, SLOT(setlevel2()));
	
	connect(setbullet3TB, SIGNAL(pressed()),
		this, SLOT(setlevel3()));
	
	connect(setbullet4TB, SIGNAL(pressed()),
		this, SLOT(setlevel4()));
	
}


BulletsModule::~BulletsModule()
{
}

void BulletsModule::standard(int row, int col)
{
	le_->setText(bullets_standard[6*row + col]);
}

void BulletsModule::maths(int row, int col)
{
	le_->setText(bullets_amssymb[6*row + col]);
}

void BulletsModule::ding1(int row, int col)
{
	le_->setText(bullets_psnfss1[6*row + col]);
}

void BulletsModule::ding2(int row, int col)
{
	le_->setText(bullets_psnfss2[6*row + col]);
}

void BulletsModule::ding3(int row, int col)
{
	le_->setText(bullets_psnfss3[6*row + col]);
}

void BulletsModule::ding4(int row, int col)
{
	le_->setText(bullets_psnfss4[6*row + col]);
}

void BulletsModule::setlevel1()
{
	le_ = bullet1LE;
}

void BulletsModule::setlevel2()
{
	le_ = bullet2LE;
}

void BulletsModule::setlevel3()
{
	le_ = bullet3LE;
}

void BulletsModule::setlevel4()
{
	le_ = bullet4LE;
}

