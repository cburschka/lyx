/**
 * \file QCharacter.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include <vector>

#include "gettext.h"
#include "support/lstrings.h"

#include "ControlCharacter.h"
#include "QCharacterDialog.h"
#include "QCharacter.h"
#include "Qt2BC.h"
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

using namespace character;
using std::vector;
 
typedef Qt2CB<ControlCharacter, Qt2DB<QCharacterDialog> > base_class;
 
QCharacter::QCharacter(ControlCharacter & c)
	: base_class(c, _("Character"))
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
	language = getLanguageData();

	for (vector<FamilyPair>::const_iterator cit = family.begin();
		cit != family.end(); ++cit) {
		dialog_->familyCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		dialog_->seriesCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		dialog_->shapeCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		dialog_->sizeCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		dialog_->miscCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		dialog_->colorCO->insertItem(cit->first.c_str(), -1);
	}
	for (vector<string>::const_iterator cit = language.begin();
		cit != language.end(); ++cit) {
		dialog_->langCO->insertItem(cit->c_str(), -1);
	}
 
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->familyCO);
	bc().addReadOnly(dialog_->seriesCO);
	bc().addReadOnly(dialog_->sizeCO);
	bc().addReadOnly(dialog_->shapeCO);
	bc().addReadOnly(dialog_->miscCO);
	bc().addReadOnly(dialog_->langCO);
	bc().addReadOnly(dialog_->colorCO);
	bc().addReadOnly(dialog_->toggleallCB);
	bc().addReadOnly(dialog_->autoapplyCB);
}


namespace {
 
template<class A, class B>
int findPos2nd(vector<std::pair<A,B> > const & vec, B const & val)
{
	vector<std::pair<A,B> >::const_iterator cit = vec.begin();
	for (; cit != vec.end(); ++cit) {
		if (cit->second == val)
			return int(cit - vec.begin()); 
	}
	return 0;
}
 
} // namespace anon
 
 
void QCharacter::update_contents()
{
	dialog_->familyCO->setCurrentItem(findPos2nd(family, controller().getFamily()));
	dialog_->seriesCO->setCurrentItem(findPos2nd(series, controller().getSeries()));
	dialog_->shapeCO->setCurrentItem(findPos2nd(shape, controller().getShape()));
	dialog_->sizeCO->setCurrentItem(findPos2nd(size, controller().getSize()));
	dialog_->miscCO->setCurrentItem(findPos2nd(bar, controller().getBar()));
	dialog_->colorCO->setCurrentItem(findPos2nd(color, controller().getColor()));

	dialog_->toggleallCB->setChecked(controller().getToggleAll());
 
	string const thelanguage(controller().getLanguage());
	int i = 0; 
	for (vector<string>::const_iterator cit = language.begin();
		cit != language.end(); ++i, ++cit) {
		if (*cit == thelanguage) {
			dialog_->langCO->setCurrentItem(i);
			break;
		}
	}
}

 
void QCharacter::apply()
{
	controller().setFamily(family[dialog_->familyCO->currentItem()].second);
	controller().setSeries(series[dialog_->seriesCO->currentItem()].second);
	controller().setShape(shape[dialog_->shapeCO->currentItem()].second);
	controller().setSize(size[dialog_->sizeCO->currentItem()].second);
	controller().setBar(bar[dialog_->miscCO->currentItem()].second);
	controller().setColor(color[dialog_->colorCO->currentItem()].second);
 
	controller().setLanguage(dialog_->langCO->currentText().latin1());
   
	controller().setToggleAll(dialog_->toggleallCB->isChecked());
}
