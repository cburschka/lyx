/**
 * \file BulletsModule.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BulletsModule.h"
#include "qt_helpers.h"

#include <QPixmap>
#include <QPainter>

using namespace std;

namespace lyx {

BulletsModule::BulletsModule(QWidget * parent)
    : QWidget(parent)
{
	setupUi(this);

	for (int iter = 0; iter < 4; ++iter)
		bullets_[iter] = ITEMIZE_DEFAULTS[iter];

	current_font_ = -1;
	current_char_ = 0;

	// add levels
	levelLW->addItem("1");
	levelLW->addItem("2");
	levelLW->addItem("3");
	levelLW->addItem("4");

	// insert pixmaps
	setupPanel(new QListWidget(bulletpaneSW), qt_("Standard[[Bullets]]"), "standard");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Maths"), "amssymb");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 1"), "psnfss1");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 2"), "psnfss2");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 3"), "psnfss3");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 4"), "psnfss4");

	connect(levelLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(showLevel(int)));
	connect(bulletpaneCO, SIGNAL(activated(int)), bulletpaneSW,
		SLOT(setCurrentIndex(int)));
}


void BulletsModule::setupPanel(QListWidget * lw, QString const & panelname,
	string const & fname)
{
	connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(bulletSelected(QListWidgetItem *, QListWidgetItem*)));

	// add panelname to combox
	bulletpaneCO->addItem(panelname);

	// get pixmap with bullets
	QPixmap pixmap(":/images/" + toqstr(fname) + ".png");

	int const w = pixmap.width() / 6;
	int const h = pixmap.height() / 6;

	// apply setting to listwidget
	lw->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lw->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lw->setViewMode(QListView::IconMode);
	lw->setFlow(QListView::LeftToRight);
	lw->setMovement(QListView::Static);
	lw->setUniformItemSizes(true);
	lw->setGridSize(QSize(w, h));
	// the widening by 21 is needed to avoid wrapping
	lw->resize(6 * w + 21, 6 * h);
	bulletpaneSW->setMinimumSize(6 * w, 6 * h + 6);

	// get individual bullets from pixmap
	for (int row = 0; row < 6; ++row) {
		for (int col = 0; col < 6; ++col) {
			QPixmap small(w, h);
			small.fill();
			QPainter painter(&small);
			painter.drawPixmap(small.rect(), pixmap, QRect(col * w, row * h, w, h));
			new QListWidgetItem(QIcon(small), "" , lw, (6 * row + col));
		}
	}

	// add bulletpanel to stackedwidget
	bulletpaneSW->addWidget(lw);
}


void BulletsModule::showLevel(int level)
{
	// unselect previous item
	selectItem(current_font_, current_char_, false);

	current_font_ = bullets_[level].getFont();

	if (bullets_[level].getFont() < 0) {
		customCB->setCheckState(Qt::Checked);
		customLE->setText(toqstr(bullets_[level].getText()));
	} else {
		customCB->setCheckState(Qt::Unchecked);
		customLE->clear();
		current_char_ = bullets_[level].getCharacter();
		selectItem(current_font_, current_char_, true);
		bulletpaneCO->setCurrentIndex(current_font_);
		bulletpaneSW->setCurrentIndex(current_font_);
	}
	bulletsizeCO->setCurrentIndex(bullets_[level].getSize() + 1);
}


void BulletsModule::init()
{
	levelLW->setCurrentRow(0);
	showLevel(0);
}


void BulletsModule::bulletSelected(QListWidgetItem * item, QListWidgetItem *)
{
	// unselect previous item
	selectItem(current_font_, current_char_, false);

	int const level = levelLW->currentRow();
	bullets_[level].setCharacter(item->type());
	bullets_[level].setFont(bulletpaneCO->currentIndex());
	current_font_ = bulletpaneCO->currentIndex();
	current_char_ = item->type();
	changed();
}


void BulletsModule::on_customCB_clicked(bool custom)
{
	if (!custom) {
		if (current_font_ < 0)
			current_font_ = bulletpaneCO->currentIndex();
		return;
	}

	// unselect previous item
	selectItem(current_font_, current_char_, false);
	current_font_ = -1;
	changed();
}


void BulletsModule::selectItem(int font, int character, bool select)
{
	if (font < 0)
		return;

	QListWidget * lw = static_cast<QListWidget *>(bulletpaneSW->widget(font));
	lw->setItemSelected(lw->item(character), select);
}


void BulletsModule::on_customLE_textEdited(const QString & text)
{
	if (customCB->checkState() == Qt::Unchecked)
		return;

	bullets_[levelLW->currentRow()].setFont(current_font_);
	bullets_[levelLW->currentRow()].setText(qstring_to_ucs4(text));
	changed();
}


void BulletsModule::on_bulletsizeCO_activated(int size)
{
	// -1 apparently means default...
	bullets_[levelLW->currentRow()].setSize(size - 1);
}


void BulletsModule::setBullet(int level, Bullet const & bullet)
{
	bullets_[level] = bullet;
}


Bullet const & BulletsModule::bullet(int level) const
{
	return bullets_[level];
}

} // namespace lyx


#include "BulletsModule_moc.cpp"
