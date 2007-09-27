/**
 * \file GuiCharacter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCharacter.h"

#include "ControlCharacter.h"
#include "qt_helpers.h"
#include "Color.h"
#include "Font.h"

#include <QCloseEvent>

using std::vector;

namespace lyx {
namespace frontend {

static vector<ShapePair> const getShapeData()
{
	vector<ShapePair> shape(6);

	ShapePair pr;

	pr.first = qt_("No change");
	pr.second = Font::IGNORE_SHAPE;
	shape[0] = pr;

	pr.first = qt_("Upright");
	pr.second = Font::UP_SHAPE;
	shape[1] = pr;

	pr.first = qt_("Italic");
	pr.second = Font::ITALIC_SHAPE;
	shape[2] = pr;

	pr.first = qt_("Slanted");
	pr.second = Font::SLANTED_SHAPE;
	shape[3] = pr;

	pr.first = qt_("Small Caps");
	pr.second = Font::SMALLCAPS_SHAPE;
	shape[4] = pr;

	pr.first = qt_("Reset");
	pr.second = Font::INHERIT_SHAPE;
	shape[5] = pr;

	return shape;
}


static vector<SizePair> const getSizeData()
{
	vector<SizePair> size(14);

	SizePair pr;

	pr.first = qt_("No change");
	pr.second = Font::IGNORE_SIZE;
	size[0] = pr;

	pr.first = qt_("Tiny");
	pr.second = Font::SIZE_TINY;
	size[1] = pr;

	pr.first = qt_("Smallest");
	pr.second = Font::SIZE_SCRIPT;
	size[2] = pr;

	pr.first = qt_("Smaller");
	pr.second = Font::SIZE_FOOTNOTE;
	size[3] = pr;

	pr.first = qt_("Small");
	pr.second = Font::SIZE_SMALL;
	size[4] = pr;

	pr.first = qt_("Normal");
	pr.second = Font::SIZE_NORMAL;
	size[5] = pr;

	pr.first = qt_("Large");
	pr.second = Font::SIZE_LARGE;
	size[6] = pr;

	pr.first = qt_("Larger");
	pr.second = Font::SIZE_LARGER;
	size[7] = pr;

	pr.first = qt_("Largest");
	pr.second = Font::SIZE_LARGEST;
	size[8] = pr;

	pr.first = qt_("Huge");
	pr.second = Font::SIZE_HUGE;
	size[9] = pr;

	pr.first = qt_("Huger");
	pr.second = Font::SIZE_HUGER;
	size[10] = pr;

	pr.first = qt_("Increase");
	pr.second = Font::INCREASE_SIZE;
	size[11] = pr;

	pr.first = qt_("Decrease");
	pr.second = Font::DECREASE_SIZE;
	size[12] = pr;

	pr.first = qt_("Reset");
	pr.second = Font::INHERIT_SIZE;
	size[13] = pr;

	return size;
}


static vector<BarPair> const getBarData()
{
	vector<BarPair> bar(5);

	BarPair pr;

	pr.first = qt_("No change");
	pr.second = IGNORE;
	bar[0] = pr;

	pr.first = qt_("Emph");
	pr.second = EMPH_TOGGLE;
	bar[1] = pr;

	pr.first = qt_("Underbar");
	pr.second = UNDERBAR_TOGGLE;
	bar[2] = pr;

	pr.first = qt_("Noun");
	pr.second = NOUN_TOGGLE;
	bar[3] = pr;

	pr.first = qt_("Reset");
	pr.second = INHERIT;
	bar[4] = pr;

	return bar;
}


static vector<ColorPair> const getColorData()
{
	vector<ColorPair> color(11);

	ColorPair pr;

	pr.first = qt_("No change");
	pr.second = Color::ignore;
	color[0] = pr;

	pr.first = qt_("No color");
	pr.second = Color::none;
	color[1] = pr;

	pr.first = qt_("Black");
	pr.second = Color::black;
	color[2] = pr;

	pr.first = qt_("White");
	pr.second = Color::white;
	color[3] = pr;

	pr.first = qt_("Red");
	pr.second = Color::red;
	color[4] = pr;

	pr.first = qt_("Green");
	pr.second = Color::green;
	color[5] = pr;

	pr.first = qt_("Blue");
	pr.second = Color::blue;
	color[6] = pr;

	pr.first = qt_("Cyan");
	pr.second = Color::cyan;
	color[7] = pr;

	pr.first = qt_("Magenta");
	pr.second = Color::magenta;
	color[8] = pr;

	pr.first = qt_("Yellow");
	pr.second = Color::yellow;
	color[9] = pr;

	pr.first = qt_("Reset");
	pr.second = Color::inherit;
	color[10] = pr;

	return color;
}


static vector<SeriesPair> const getSeriesData()
{
	vector<SeriesPair> series(4);

	SeriesPair pr;

	pr.first = qt_("No change");
	pr.second = Font::IGNORE_SERIES;
	series[0] = pr;

	pr.first = qt_("Medium");
	pr.second = Font::MEDIUM_SERIES;
	series[1] = pr;

	pr.first = qt_("Bold");
	pr.second = Font::BOLD_SERIES;
	series[2] = pr;

	pr.first = qt_("Reset");
	pr.second = Font::INHERIT_SERIES;
	series[3] = pr;

	return series;
}


static vector<FamilyPair> const getFamilyData()
{
	vector<FamilyPair> family(5);

	FamilyPair pr;

	pr.first = qt_("No change");
	pr.second = Font::IGNORE_FAMILY;
	family[0] = pr;

	pr.first = qt_("Roman");
	pr.second = Font::ROMAN_FAMILY;
	family[1] = pr;

	pr.first = qt_("Sans Serif");
	pr.second = Font::SANS_FAMILY;
	family[2] = pr;

	pr.first = qt_("Typewriter");
	pr.second = Font::TYPEWRITER_FAMILY;
	family[3] = pr;

	pr.first = qt_("Reset");
	pr.second = Font::INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


GuiCharacterDialog::GuiCharacterDialog(LyXView & lv)
	: GuiDialog(lv, "character")
{
	setupUi(this);
	setController(new ControlCharacter(*this));
	setViewTitle(_("Text Style"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(miscCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(sizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(familyCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(seriesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(shapeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(colorCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(toggleallCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	family = getFamilyData();
	series = getSeriesData();
	shape  = getShapeData();
	size   = getSizeData();
	bar    = getBarData();
	color  = getColorData();
	language = getLanguageData(true);

	for (vector<FamilyPair>::const_iterator cit = family.begin();
		cit != family.end(); ++cit) {
		familyCO->addItem(cit->first);
	}

	for (vector<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		seriesCO->addItem(cit->first);
	}
	for (vector<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		shapeCO->addItem(cit->first);
	}
	for (vector<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		sizeCO->addItem(cit->first);
	}
	for (vector<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		miscCO->addItem(cit->first);
	}
	for (vector<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		colorCO->addItem(cit->first);
	}
	for (vector<LanguagePair>::const_iterator cit = language.begin();
		cit != language.end(); ++cit) {
		langCO->addItem(toqstr(cit->first));
	}

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().addReadOnly(familyCO);
	bc().addReadOnly(seriesCO);
	bc().addReadOnly(sizeCO);
	bc().addReadOnly(shapeCO);
	bc().addReadOnly(miscCO);
	bc().addReadOnly(langCO);
	bc().addReadOnly(colorCO);
	bc().addReadOnly(toggleallCB);
	bc().addReadOnly(autoapplyCB);

// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	// qt resizes the comboboxes only after show(), so ...
	QDialog::show();
#endif
}


ControlCharacter & GuiCharacterDialog::controller()
{
	return static_cast<ControlCharacter &>(GuiDialog::controller());
}


void GuiCharacterDialog::change_adaptor()
{
	changed();

	if (!autoapplyCB->isChecked())
		return;

	// to be really good here, we should set the combos to the values of
	// the current text, and make it appear as "no change" if the values
	// stay the same between applys. Might be difficult though wrt to a
	// moved cursor - jbl
	slotApply();
	familyCO->setCurrentIndex(0);
	seriesCO->setCurrentIndex(0);
	sizeCO->setCurrentIndex(0);
	shapeCO->setCurrentIndex(0);
	miscCO->setCurrentIndex(0);
	langCO->setCurrentIndex(0);
	colorCO->setCurrentIndex(0);
}


void GuiCharacterDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	GuiDialog::closeEvent(e);
}


template<class A, class B>
static int findPos2nd(vector<std::pair<A, B> > const & vec, B const & val)
{
	typedef typename vector<std::pair<A, B> >::const_iterator
		const_iterator;

	for (const_iterator cit = vec.begin(); cit != vec.end(); ++cit)
		if (cit->second == val)
			return int(cit - vec.begin());

	return 0;
}


void GuiCharacterDialog::updateContents()
{
	ControlCharacter const & ctrl = controller();

	familyCO->setCurrentIndex(findPos2nd(family, ctrl.getFamily()));
	seriesCO->setCurrentIndex(findPos2nd(series, ctrl.getSeries()));
	shapeCO->setCurrentIndex(findPos2nd(shape, ctrl.getShape()));
	sizeCO->setCurrentIndex(findPos2nd(size, ctrl.getSize()));
	miscCO->setCurrentIndex(findPos2nd(bar, ctrl.getBar()));
	colorCO->setCurrentIndex(findPos2nd(color, ctrl.getColor()));
	langCO->setCurrentIndex(findPos2nd(language, ctrl.getLanguage()));

	toggleallCB->setChecked(ctrl.getToggleAll());
}


void GuiCharacterDialog::applyView()
{
	ControlCharacter & ctrl = controller();

	ctrl.setFamily(family[familyCO->currentIndex()].second);
	ctrl.setSeries(series[seriesCO->currentIndex()].second);
	ctrl.setShape(shape[shapeCO->currentIndex()].second);
	ctrl.setSize(size[sizeCO->currentIndex()].second);
	ctrl.setBar(bar[miscCO->currentIndex()].second);
	ctrl.setColor(color[colorCO->currentIndex()].second);
	ctrl.setLanguage(language[langCO->currentIndex()].second);

	ctrl.setToggleAll(toggleallCB->isChecked());
}

} // namespace frontend
} // namespace lyx

#include "GuiCharacter_moc.cpp"
