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
#include "frontend_helpers.h"
#include "Color.h"

#include <QCloseEvent>

using std::vector;

namespace lyx {
namespace frontend {

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
		familyCO->addItem(toqstr(cit->first));
	}

	for (vector<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		seriesCO->addItem(toqstr(cit->first));
	}
	for (vector<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		shapeCO->addItem(toqstr(cit->first));
	}
	for (vector<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		sizeCO->addItem(toqstr(cit->first));
	}
	for (vector<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		miscCO->addItem(toqstr(cit->first));
	}
	for (vector<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		colorCO->addItem(toqstr(cit->first));
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


ControlCharacter & GuiCharacterDialog::controller() const
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
	e->accept();
}


template<class A, class B>
static int findPos2nd(vector<std::pair<A,B> > const & vec, B const & val)
{
	typedef typename vector<std::pair<A, B> >::const_iterator
		const_iterator;

	const_iterator cit = vec.begin();
	for (; cit != vec.end(); ++cit) {
		if (cit->second == val)
			return int(cit - vec.begin());
	}
	return 0;
}


void GuiCharacterDialog::update_contents()
{
	ControlCharacter const & ctrl = controller();

	familyCO->setCurrentIndex(findPos2nd(family,
						     ctrl.getFamily()));
	seriesCO->setCurrentIndex(findPos2nd(series,
						     ctrl.getSeries()));
	shapeCO->setCurrentIndex(findPos2nd(shape, ctrl.getShape()));
	sizeCO->setCurrentIndex(findPos2nd(size, ctrl.getSize()));
	miscCO->setCurrentIndex(findPos2nd(bar, ctrl.getBar()));
	colorCO->setCurrentIndex(findPos2nd(color, ctrl.getColor()));
	langCO->setCurrentIndex(findPos2nd(language,
						   ctrl.getLanguage()));

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
