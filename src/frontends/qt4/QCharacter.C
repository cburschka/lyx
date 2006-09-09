/**
 * \file QCharacter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCharacter.h"
#include "ControlCharacter.h"
#include "QCharacterDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "LColor.h"

#include <qpushbutton.h>
#include <qcheckbox.h>

using std::vector;

namespace lyx {
namespace frontend {

typedef QController<ControlCharacter, QView<QCharacterDialog> > base_class;


QCharacter::QCharacter(Dialog & parent)
	: base_class(parent, lyx::to_utf8(_("Text Style")))
{
}


void QCharacter::build_dialog()
{
	dialog_.reset(new QCharacterDialog(this));

	family = getFamilyData();
	series = getSeriesData();
	shape  = getShapeData();
	size   = getSizeData();
	bar    = getBarData();
	color  = getColorData();
	language = getLanguageData(true);

	for (vector<FamilyPair>::const_iterator cit = family.begin();
		cit != family.end(); ++cit) {
		dialog_->familyCO->addItem(toqstr(cit->first));
	}

	for (vector<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		dialog_->seriesCO->addItem(toqstr(cit->first));
	}
	for (vector<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		dialog_->shapeCO->addItem(toqstr(cit->first));
	}
	for (vector<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		dialog_->sizeCO->addItem(toqstr(cit->first));
	}
	for (vector<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		dialog_->miscCO->addItem(toqstr(cit->first));
	}
	for (vector<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		dialog_->colorCO->addItem(toqstr(cit->first));
	}
	for (vector<LanguagePair>::const_iterator cit = language.begin();
		cit != language.end(); ++cit) {
		dialog_->langCO->addItem(toqstr(cit->first));
	}

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->familyCO);
	bcview().addReadOnly(dialog_->seriesCO);
	bcview().addReadOnly(dialog_->sizeCO);
	bcview().addReadOnly(dialog_->shapeCO);
	bcview().addReadOnly(dialog_->miscCO);
	bcview().addReadOnly(dialog_->langCO);
	bcview().addReadOnly(dialog_->colorCO);
	bcview().addReadOnly(dialog_->toggleallCB);
	bcview().addReadOnly(dialog_->autoapplyCB);
}


namespace {

template<class A, class B>
int findPos2nd(vector<std::pair<A,B> > const & vec, B const & val)
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

} // namespace anon


void QCharacter::update_contents()
{
	ControlCharacter const & ctrl = controller();

	dialog_->familyCO->setCurrentIndex(findPos2nd(family,
						     ctrl.getFamily()));
	dialog_->seriesCO->setCurrentIndex(findPos2nd(series,
						     ctrl.getSeries()));
	dialog_->shapeCO->setCurrentIndex(findPos2nd(shape, ctrl.getShape()));
	dialog_->sizeCO->setCurrentIndex(findPos2nd(size, ctrl.getSize()));
	dialog_->miscCO->setCurrentIndex(findPos2nd(bar, ctrl.getBar()));
	dialog_->colorCO->setCurrentIndex(findPos2nd(color, ctrl.getColor()));
	dialog_->langCO->setCurrentIndex(findPos2nd(language,
						   ctrl.getLanguage()));

	dialog_->toggleallCB->setChecked(ctrl.getToggleAll());
}


void QCharacter::apply()
{
	ControlCharacter & ctrl = controller();

	ctrl.setFamily(family[dialog_->familyCO->currentIndex()].second);
	ctrl.setSeries(series[dialog_->seriesCO->currentIndex()].second);
	ctrl.setShape(shape[dialog_->shapeCO->currentIndex()].second);
	ctrl.setSize(size[dialog_->sizeCO->currentIndex()].second);
	ctrl.setBar(bar[dialog_->miscCO->currentIndex()].second);
	ctrl.setColor(color[dialog_->colorCO->currentIndex()].second);
	ctrl.setLanguage(language[dialog_->langCO->currentIndex()].second);

	ctrl.setToggleAll(dialog_->toggleallCB->isChecked());
}

} // namespace frontend
} // namespace lyx
