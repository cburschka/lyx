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

#include "BulletsModule.h"
// #include "QBrowseBox.h"
#include "qt_helpers.h"

#include "support/filetools.h"

#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QComboBox>
#include <QPixmap>

#include <boost/assert.hpp>

using lyx::support::libFileSearch;

using std::string;


BulletsModule::BulletsModule(QWidget * parent,  const char * name, Qt::WFlags fl)
	: bullet_pressed_(0)
{
	setupUi(this);

	for (int iter = 0; iter < 4; ++iter) {
		bullets_[iter] = ITEMIZE_DEFAULTS[iter];
	}

	QMenu * pm = new QMenu(this);

	QMenu * pm1 = new QMenu(pm);
	QMenu * pm2 = new QMenu(pm);
	QMenu * pm3 = new QMenu(pm);
	QMenu * pm4 = new QMenu(pm);
	QMenu * pm5 = new QMenu(pm);
	QMenu * pm6 = new QMenu(pm);

// FIXME: We need a Qt4 compatible browsebox type widget
// which can act as a popup to a toolbutton
/*
	standard_ = new QBrowseBox(6, 6, pm1);
	maths_ = new QBrowseBox(6, 6, pm2);
	ding1_ = new QBrowseBox(6, 6, pm3);
	ding2_ = new QBrowseBox(6, 6, pm4);
	ding3_ = new QBrowseBox(6, 6, pm5);
	ding4_ = new QBrowseBox(6, 6, pm6);

	///\todo See how to insert those BrowseBox:
	pm1->addMenu((QMenu*) standard_);
	pm2->addMenu((QMenu*)maths_);
	pm3->addMenu((QMenu*)ding1_);
	pm4->addMenu((QMenu*)ding2_);
	pm5->addMenu((QMenu*)ding3_);
	pm6->addMenu((QMenu*)ding4_);

	pm->insertItem(qt_("&Standard"), pm1, 0);
	pm->insertItem(qt_("&Maths"), pm2, 1);
	pm->insertItem(qt_("Dings &1"), pm3, 2);
	pm->insertItem(qt_("Dings &2"), pm4, 3);
	pm->insertItem(qt_("Dings &3"), pm5, 4);
	pm->insertItem(qt_("Dings &4"), pm6, 5);
	pm->insertSeparator();
	// FIXME: make this checkable
	pm->insertItem(qt_("&Custom..."), this, SLOT(setCustom()), 0, 6);

	connect(bullet1PB, SIGNAL(pressed()), this, SLOT(clicked1()));
	bullet1PB->setPopup(pm);

	connect(bullet2PB, SIGNAL(pressed()), this, SLOT(clicked2()));
	bullet2PB->setPopup(pm);

	connect(bullet3PB, SIGNAL(pressed()), this, SLOT(clicked3()));
	bullet3PB->setPopup(pm);

	connect(bullet4PB, SIGNAL(pressed()), this, SLOT(clicked4()));
	bullet4PB->setPopup(pm);

	// insert pixmaps
	string bmfile;
	bmfile = libFileSearch("images", "standard", "xpm");
	standard_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = libFileSearch("images", "amssymb", "xpm");
	maths_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = libFileSearch("images", "psnfss1", "xpm");
	ding1_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = libFileSearch("images", "psnfss2", "xpm");
	ding2_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = libFileSearch("images", "psnfss3", "xpm");
	ding3_->insertItem(QPixmap(toqstr(bmfile)));

	bmfile = libFileSearch("images", "psnfss4", "xpm");
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

	connect(bullet1PB, SIGNAL(pressed()), this, SLOT(selected1()));
	connect(bullet2PB, SIGNAL(pressed()), this, SLOT(selected2()));
	connect(bullet3PB, SIGNAL(pressed()), this, SLOT(selected3()));
	connect(bullet4PB, SIGNAL(pressed()), this, SLOT(selected4()));
	connect(bulletsize1CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize2CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize3CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));
	connect(bulletsize4CO, SIGNAL(activated(int)), this, SLOT(updateSizes()));

	// update the view
	for (int i = 0; i < 4; ++i)
		setBullet(bullet1PB, bulletsize1CO, bullets_[i]);
*/
}


BulletsModule::~BulletsModule()
{
}


void BulletsModule::updateSizes()
{
        // emit signal
	changed();

	// -1 apparently means default...
	bullets_[0].setSize(bulletsize1CO->currentItem() - 1);
	bullets_[1].setSize(bulletsize2CO->currentItem() - 1);
	bullets_[2].setSize(bulletsize3CO->currentItem() - 1);
	bullets_[3].setSize(bulletsize4CO->currentItem() - 1);
}


// These arrive before the menus are launched.
void BulletsModule::clicked1()
{
	bullet_pressed_ = &bullets_[0];
}


void BulletsModule::clicked2()
{
	bullet_pressed_ = &bullets_[1];
}


void BulletsModule::clicked3()
{
	bullet_pressed_ = &bullets_[2];
}


void BulletsModule::clicked4()
{
	bullet_pressed_ = &bullets_[3];
}


// These arrive *after* the menus have done their work
void BulletsModule::selected1()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[0] = tmpbullet;
	setBullet(bullet1PB, bulletsize1CO, bullets_[0]);
        // emit signal
	changed();
}


void BulletsModule::selected2()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[1] = tmpbullet;
	setBullet(bullet2PB, bulletsize2CO, bullets_[1]);
        // emit signal
	changed();
}


void BulletsModule::selected3()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[2] = tmpbullet;
	setBullet(bullet3PB, bulletsize3CO, bullets_[2]);
        // emit signal
	changed();
}


void BulletsModule::selected4()
{
	if (!tmpbulletset)
		return;
	tmpbulletset = false;
	bullets_[3] = tmpbullet;
	setBullet(bullet4PB, bulletsize4CO, bullets_[3]);
        // emit signal
	changed();
}


QPixmap BulletsModule::getPixmap(int font, int character)
{
	int col = character % 6;
	int row = (character - col) / 6;

/*	switch (font) {
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
	}*/
	// make qt4 frontend at least compilable now.
	return QPixmap();
}


void BulletsModule::setBullet(QPushButton * pb, QComboBox * co, Bullet const & b)
{
	if (b.getFont() == -1) {
		pb->setPixmap(QPixmap());
		pb->setText(toqstr(b.getText()));
	} else {
		pb->setPixmap(getPixmap(b.getFont(), b.getCharacter()));
	}

	pb->setMinimumSize(QSize(50, 50));

	co->setCurrentItem(b.getSize() + 1);
}


void BulletsModule::setBullet(int level, const Bullet & bullet)
{
	bullets_[level] = bullet;
	// FIXME: backout until we fixed the browsebox
	return;

	QPushButton * pb = 0;
	QComboBox * co = 0;

	switch (level) {
		case 0: pb = bullet1PB; co = bulletsize1CO; break;
		case 1: pb = bullet2PB; co = bulletsize2CO; break;
		case 2: pb = bullet3PB; co = bulletsize3CO; break;
		case 3: pb = bullet4PB; co = bulletsize4CO; break;
		default: BOOST_ASSERT(false); break;
	}

	setBullet(pb, co, bullet);
}


Bullet const & BulletsModule::getBullet(int level) const
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
	QString const input = bullet_pressed_ ?
		toqstr(bullet_pressed_->getText()) : QString::null;

	bool ok = false;
	QString text = QInputDialog::getText(
		qt_( "Bullets" ),
		qt_( "Enter a custom bullet" ),
		QLineEdit::Normal,
		input, &ok, this );

	if (!ok)
		return;

	tmpbulletset = true;
	tmpbullet.setText(fromqstr(text));
	tmpbullet.setFont(-1);
}

#include "BulletsModule_moc.cpp"
