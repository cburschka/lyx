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
#include "qt_helpers.h"

#include <functional> // for operator %

#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qlabel.h>
#include "BulletsModule.h"
#include "Bullet.h"
#include "ui/BulletsModuleBase.h"
#include "QBrowseBox.h"
#include "support/filetools.h"

BulletsModule::BulletsModule(QWidget * parent,  char const * name, WFlags fl)
	: BulletsModuleBase(parent, name, fl)
{
	for (int iter = 0; iter < 4; ++iter) {
		bullets_[iter] = ITEMIZE_DEFAULTS[iter];
	}
	
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
	pm->insertItem("Custom...", this, SLOT(setCustom()));

	setbulletTB->setPopup(pm);

	// insert pixmaps
	string bmfile;
	bmfile = LibFileSearch("images", "standard", "xpm");
	standard_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = LibFileSearch("images", "amssymb", "xpm");
	maths_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = LibFileSearch("images", "psnfss1", "xpm");
	ding1_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = LibFileSearch("images", "psnfss2", "xpm");
	ding2_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = LibFileSearch("images", "psnfss3", "xpm");
	ding3_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = LibFileSearch("images", "psnfss4", "xpm");
	ding4_->insertItem(QPixmap(toqstr(bmfile)));

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

	// update the view
	for (int iter = 0; iter < 4; ++iter) {
		setBullet(iter,bullets_[iter]);
	}
	activeitem_ = bulletsLV->firstChild();
	activebullet_ = &bullets_[0];
}


BulletsModule::~BulletsModule()
{
}


QPixmap BulletsModule::getPixmap(int font, int character)
{
	int col = character%6;
	int row = (character - col)/6;
	switch(font) {
	case 0:
		return standard_->pixmap(row,col);
	case 1:
		return maths_->pixmap(row,col);
	case 2:
		return ding1_->pixmap(row,col);
	case 3:
		return ding2_->pixmap(row,col);
	case 4:
		return ding3_->pixmap(row,col);
	case 5:
		return ding4_->pixmap(row,col);
	default:
		return standard_->pixmap(row,col);
	}
}


QListViewItem *  BulletsModule::getItem(int level)
{
	QListViewItemIterator it(bulletsLV->firstChild());
	for(int i=0; i<level; ++i) {
		++it;
	}
	return it.current();
}


void BulletsModule::setActive(int level)
{
	activeitem_ = getItem(level);
	activebullet_ = &bullets_[level];
}


void BulletsModule::setActive(QListViewItem * item)
{
	activeitem_ = item;
	activebullet_ = &bullets_[item->depth()];
}


void BulletsModule::setBullet(int font, int character)
{
	activeitem_->setText(0,"");
	activeitem_->setPixmap(0,getPixmap(font,character));
	
	activebullet_->setFont(font);
	activebullet_->setCharacter(character);
}


void BulletsModule::setBullet(string text)
{
	activeitem_->setPixmap(0, QPixmap());
	activeitem_->setText(0, toqstr(text));
	
	activebullet_->setText(text);
}


void BulletsModule::setBullet(int level, const Bullet & bullet)
{
	setActive(level);
	bullets_[level] = bullet;
	// set size
	setSize(bullet.getSize()+1);
	// set pixmap
	if (bullet.getFont()!=-1) {
		setBullet(bullet.getFont(),
			  bullet.getCharacter());
	} else {
		setBullet(bullet.getText());
	}
}


Bullet BulletsModule::getBullet(int level)
{
	return bullets_[level];
}


void BulletsModule::setSize(int size)
{
	activeitem_->setText(1,bulletsizeCO->text(size));
	activebullet_->setSize(size-1);
}


void BulletsModule::standard(int row, int col)
{
	setBullet(0,6*row + col);
}

void BulletsModule::maths(int row, int col)
{
	setBullet(1,6*row + col);
}


void BulletsModule::ding1(int row, int col)
{
	setBullet(2,6*row + col);
}


void BulletsModule::ding2(int row, int col)
{
	setBullet(3,6*row + col);
}


void BulletsModule::ding3(int row, int col)
{
	setBullet(4,6*row + col);
}	


void BulletsModule::ding4(int row, int col)
{
	setBullet(5,6*row + col);
}


void BulletsModule::setCustom()
{
	bool ok = FALSE;
	QString text = QInputDialog::getText(
		qt_( "Bullets" ),
		qt_( "Enter a custom bullet" ),
		QLineEdit::Normal,
		QString::null, &ok, this );

	if (ok) {
		activeitem_->setPixmap(0,QPixmap());
		activeitem_->setText(0,text);
		activebullet_->setText(fromqstr(text));
		activebullet_->setFont(-1);
	}
}
