/**
 * \file BulletsModule.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"

#include "support/filetools.h"

#include "QBrowseBox.h"
#include "BulletsModule.h"

#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <boost/assert.hpp>

using lyx::support::LibFileSearch;


BulletsModule::BulletsModule(QWidget * parent,  const char * name, WFlags fl)
	: BulletsModuleBase(parent, name, fl), tmpbulletset(0)
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

	pm->insertItem(qt_("&Standard"), pm1, 0);
	pm->insertItem(qt_("&Maths"), pm2, 1);
	pm->insertItem(qt_("Dings &1"), pm3, 2);
	pm->insertItem(qt_("Dings &2"), pm4, 3);
	pm->insertItem(qt_("Dings &3"), pm5, 4);
	pm->insertItem(qt_("Dings &4"), pm6, 5);
	pm->insertSeparator();
	// FIXME: make this checkable
	pm->insertItem(qt_("&Custom..."), this, SLOT(setCustom()), 0, 6);

	bullet1PB->setPopup(pm);
	bullet2PB->setPopup(pm);
	bullet3PB->setPopup(pm);
	bullet4PB->setPopup(pm);

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

	connect(bullet1PB, SIGNAL(pressed()), this, SLOT(pressed1()));
	connect(bullet2PB, SIGNAL(pressed()), this, SLOT(pressed2()));
	connect(bullet3PB, SIGNAL(pressed()), this, SLOT(pressed3()));
	connect(bullet4PB, SIGNAL(pressed()), this, SLOT(pressed4()));
	connect(bulletsize1CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize2CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize3CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize4CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));

	// update the view
	for (int i = 0; i < 4; ++i)
		setBullet(bullet1PB, bulletsize1CO, bullets_[i]);
}


BulletsModule::~BulletsModule()
{
}


void BulletsModule::updateSizes()
{
	emit changed();

	// -1 apparently means default...
	bullets_[0].setSize(bulletsize1CO->currentItem() - 1);
	bullets_[1].setSize(bulletsize2CO->currentItem() - 1);
	bullets_[2].setSize(bulletsize3CO->currentItem() - 1);
	bullets_[3].setSize(bulletsize4CO->currentItem() - 1);
}


// These arrive *after* the menus have done their work
void BulletsModule::pressed1()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[0] = tmpbullet;
	setBullet(bullet1PB, bulletsize1CO, bullets_[0]);
	emit changed();
}


void BulletsModule::pressed2()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[1] = tmpbullet;
	setBullet(bullet2PB, bulletsize2CO, bullets_[1]);
	emit changed();
}


void BulletsModule::pressed3()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[2] = tmpbullet;
	setBullet(bullet3PB, bulletsize3CO, bullets_[2]);
	emit changed();
}


void BulletsModule::pressed4()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[3] = tmpbullet;
	setBullet(bullet4PB, bulletsize4CO, bullets_[3]);
	emit changed();
}


QPixmap BulletsModule::getPixmap(int font, int character)
{
	int col = character % 6;
	int row = (character - col) / 6;

	switch (font) {
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


void BulletsModule::setBullet(QPushButton * pb, QComboBox * co, Bullet const & b)
{
	if (b.getFont() == -1) {
		pb->setPixmap(QPixmap());
		pb->setText("...");
	} else {
		pb->setPixmap(getPixmap(b.getFont(), b.getCharacter()));
	}

	pb->setMinimumSize(QSize(50, 50));

	co->setCurrentItem(b.getSize() + 1);
}


void BulletsModule::setBullet(int level, const Bullet & bullet)
{
	bullets_[level] = bullet;

	QPushButton * pb;
	QComboBox * co;

	switch (level) {
		case 0: pb = bullet1PB; co = bulletsize1CO; break;
		case 1: pb = bullet2PB; co = bulletsize2CO; break;
		case 2: pb = bullet3PB; co = bulletsize3CO; break;
		case 3: pb = bullet4PB; co = bulletsize4CO; break;
		default: BOOST_ASSERT(false); break;
	}

	setBullet(pb, co, bullet);
}


Bullet const BulletsModule::getBullet(int level)
{
	return bullets_[level];
}


void BulletsModule::setCurrentBullet(int font, int character)
{
	tmpbulletset = true;
	tmpbullet.setFont(font);
	tmpbullet.setCharacter(character);
}


void BulletsModule::standard(int row, int col)
{
	setCurrentBullet(0, 6 * row + col);
}


void BulletsModule::maths(int row, int col)
{
	setCurrentBullet(1, 6 * row + col);
}


void BulletsModule::ding1(int row, int col)
{
	setCurrentBullet(2, 6 * row + col);
}


void BulletsModule::ding2(int row, int col)
{
	setCurrentBullet(3, 6 * row + col);
}


void BulletsModule::ding3(int row, int col)
{
	setCurrentBullet(4, 6 * row + col);
}


void BulletsModule::ding4(int row, int col)
{
	setCurrentBullet(5, 6 * row + col);
}


void BulletsModule::setCustom()
{
	bool ok = FALSE;
	QString text = QInputDialog::getText(
		qt_( "Bullets" ),
		qt_( "Enter a custom bullet" ),
		QLineEdit::Normal,
		QString::null, &ok, this );

	if (!ok)
		return;

	tmpbulletset = true;
	tmpbullet.setText(fromqstr(text));
	tmpbullet.setFont(-1);
}
