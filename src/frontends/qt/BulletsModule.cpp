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

#include "Bullet.h"

#include "BulletsModule.h"
#include "GuiApplication.h"
#include "qt_helpers.h"

#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QScrollBar>

using namespace std;

namespace lyx {

namespace frontend {

int const CHARMAX = 36;
int const FONTMAX = 6;

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
	setupPanel(new QListWidget(bulletpaneSW), qt_("Standard[[Bullets]]"), 0);
	setupPanel(new QListWidget(bulletpaneSW), qt_("Maths"), 1, "math");
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 1"), 2);
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 2"), 3);
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 3"), 4);
	setupPanel(new QListWidget(bulletpaneSW), qt_("Dings 4"), 5);

	connect(levelLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(showLevel(int)));
	connect(bulletpaneCO, SIGNAL(activated(int)), bulletpaneSW,
		SLOT(setCurrentIndex(int)));
}


string const bulletIcon(int f, int c)
{
	static char const * BulletPanelIcons0[CHARMAX] = {
		/* standard */
		"bfendash", "vdash", "dashv", "flat", "natural",
		"sharp", "ast", "star", "bullet", "circ", "cdot",
		"dagger", "bigtriangleup", "bigtriangledown", "triangleleft",
		"triangleright", "lhd", "rhd", "oplus", "ominus", "otimes",
		"oslash", "odot", "spadesuit", "diamond", "diamond2", "box",
		"diamondsuit", "heartsuit",  "clubsuit", "rightarrow", "leadsto",
		"rightharpoonup", "rightharpoondown", "rightarrow2", "succ"
	};
	static char const * BulletPanelIcons1[CHARMAX] = {
		/* amssymb */
		"Rrightarrow", "rightarrowtail", "twoheadrightarrow", "rightsquigarrow",
		"looparrowright", "multimap", "boxtimes", "boxplus", "boxminus",
		"boxdot", "divideontimes", "Vvdash", "lessdot", "gtrdot", "maltese",
		"bigstar", "checkmark", "vdash2", "backsim", "thicksim",
		"centerdot", "circleddash", "circledast", "circledcirc",
		"vartriangleleft", "vartriangleright", "vartriangle", "triangledown",
		"lozenge", "square", "blacktriangleleft", "blacktriangleright", "blacktriangle",
		"blacktriangledown", "blacklozenge", "blacksquare"
	};
	static char const * BulletPanelIcons2[CHARMAX] = {
		/* psnfss1 */
		"ding108", "ding109", "ding119", "pisymbol_psy197",
		"pisymbol_psy196", "pisymbol_psy183", "ding71", "ding70",
		"ding118", "ding117", "pisymbol_psy224", "pisymbol_psy215",
		"ding111", "ding112", "ding113", "ding114",
		"pisymbol_psy68", "pisymbol_psy209", "ding120", "ding121",
		"ding122", "ding110", "ding115", "ding116",
		"pisymbol_psy42", "ding67", "ding66", "ding82",
		"ding81", "ding228", "ding162", "ding163",
		"ding166", "ding167", "ding226", "ding227"
	};
	static char const * BulletPanelIcons3[CHARMAX] = {
		/* psnfss2 */
		"ding37", "ding38", "ding34", "ding36",
		"ding39", "ding40", "ding41", "ding42",
		"ding43", "ding44", "ding45", "ding47",
		"ding53", "ding54", "ding59", "ding57",
		"ding62", "ding61", "ding55", "ding56",
		"ding58", "ding60", "ding63", "ding64",
		"ding51", "ding52", "pisymbol_psy170", "pisymbol_psy167",
		"pisymbol_psy168", "pisymbol_psy169",  "ding164", "ding165",
		"ding171", "ding168", "ding169", "ding170"
	};
	static char const * BulletPanelIcons4[CHARMAX] = {
		/* psnfss3 */
		"ding65", "ding76", "ding75", "ding72",
		"ding80", "ding74", "ding78", "ding77",
		"ding79", "ding85", "ding90", "ding98",
		"ding83", "ding84", "ding86", "ding87",
		"ding88", "ding89", "ding92", "ding91",
		"ding93", "ding105", "ding94", "ding99",
		"ding103", "ding104", "ding106", "ding107",
		"ding68", "ding69", "ding100", "ding101",
		"ding102", "ding96", "ding95", "ding97"
	};
	static char const * BulletPanelIcons5[CHARMAX] = {
		/* psnfss4 */
		"ding223", "ding224", "ding225", "ding232",
		"ding229", "ding230", "ding238", "ding237",
		"ding236", "ding235", "ding234", "ding233",
		"ding239", "ding241", "ding250", "ding251",
		"ding49", "ding50", "ding217", "ding245",
		"ding243", "ding248", "ding252", "ding253",
		"ding219", "ding213", "ding221", "ding222",
		"ding220", "ding212", "pisymbol_psy174", "pisymbol_psy222",
		"ding254", "ding242", "ding231", "pisymbol_psy45"
	};

	static char const ** BulletPanelIcons[FONTMAX] = {
		BulletPanelIcons0, BulletPanelIcons1,
		BulletPanelIcons2, BulletPanelIcons3,
		BulletPanelIcons4, BulletPanelIcons5
	};

	return BulletPanelIcons[f][c];
}


QPixmap getSelectedPixmap(QPixmap pixmap, QSize const icon_size)
{
	QPalette palette = QPalette();
	QColor text_color = (guiApp->isInDarkMode())
			? palette.color(QPalette::Active, QPalette::WindowText)
			: Qt::black;
	QColor highlight_color = palette.color(QPalette::Active, QPalette::HighlightedText);
	QColor highlight_bg = palette.color(QPalette::Active, QPalette::Highlight);

	// create a layer with black text turned to QPalette::HighlightedText
	QPixmap hl_overlay(pixmap.size());
	hl_overlay.fill(highlight_color);
	hl_overlay.setMask(pixmap.createMaskFromColor(text_color, Qt::MaskOutColor));

	// Create highlighted background
	QPixmap hl_background(icon_size);
	hl_background.fill(highlight_bg);

	// put layers on top of existing pixmap
	QPainter painter(&pixmap);
	painter.drawPixmap(pixmap.rect(), hl_background);
	painter.drawPixmap(pixmap.rect(), hl_overlay);

	return pixmap;
}


void BulletsModule::setupPanel(QListWidget * lw, QString const & panelname,
	int const font, string const folder)
{
	connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(bulletSelected(QListWidgetItem *, QListWidgetItem*)));

	// add panelname to combox
	bulletpaneCO->addItem(panelname);

	lw->setViewMode(QListView::IconMode);
	lw->setDragDropMode(QAbstractItemView::NoDragDrop);
	lw->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lw->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lw->setFlow(QListView::LeftToRight);
	lw->setSpacing(12);
	lw->setUniformItemSizes(true);

	QSize icon_size(26, 26);
	lw->setIconSize(icon_size);

	// we calculate the appropriate width to fit 6 icons in a row
	lw->setFixedWidth((6 * (icon_size.width() + lw->spacing()))
				+ (lw->frameWidth() * 2)
				+ lw->verticalScrollBar()->height());
	// and six rows
	lw->setFixedHeight((6 * (icon_size.height() + lw->spacing()))
				+ (lw->frameWidth() * 2)
				+ lw->horizontalScrollBar()->height());

	for (int i = 0; i < CHARMAX; ++i) {
		string const iconname = bulletIcon(font, i);
		// get pixmap with bullets
		QPixmap pixmap = getPixmap("images/" + toqstr(folder) + "/", toqstr(iconname), "svgz");
		QIcon icon(pixmap);
		icon.addPixmap(getSelectedPixmap(pixmap, icon_size), QIcon::Selected);
		QListWidgetItem * lwi = new QListWidgetItem(icon, QString(), 0, i);
		lwi->setToolTip(toqstr(Bullet::bulletEntry(font, i)));
		lwi->setSizeHint(icon_size);
		lw->addItem(lwi);
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
		bulletsizeCO->setEnabled(false);
		sizeL->setEnabled(false);
	} else {
		customCB->setCheckState(Qt::Unchecked);
		customLE->clear();
		current_char_ = bullets_[level].getCharacter();
		selectItem(current_font_, current_char_, true);
		bulletpaneCO->setCurrentIndex(current_font_);
		bulletpaneSW->setCurrentIndex(current_font_);
		bulletsizeCO->setEnabled(true);
		sizeL->setEnabled(true);
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
	bulletsizeCO->setEnabled(!custom);
	sizeL->setEnabled(!custom);

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
	if (lw->item(character))
		lw->item(character)->setSelected(select);
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
	changed();
}


void BulletsModule::setBullet(int level, Bullet const & bullet)
{
	bullets_[level] = bullet;
}


Bullet const & BulletsModule::bullet(int level) const
{
	return bullets_[level];
}

} // namespace frontend

} // namespace lyx


#include "moc_BulletsModule.cpp"
