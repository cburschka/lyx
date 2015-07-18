/**
 * \file LengthCombo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/qstring_helpers.h"

#include "LengthCombo.h"

#include "qt_helpers.h"

#include <string>


namespace lyx {
namespace frontend {

LengthCombo::LengthCombo(QWidget * parent)
	: QComboBox(parent)
{
	for (int i = 0; i < lyx::num_units; i++) {
		// mu does not make sense usually
		// so it must be added manually, if needed
		if (QLatin1String(lyx::unit_name[i]) == "mu")
			continue;
		QComboBox::addItem(lyx::qt_(lyx::unit_name_gui[i]),
			lyx::toqstr(lyx::unit_name[i]));
	}

	connect(this, SIGNAL(activated(int)),
		this, SLOT(hasActivated(int)));
}


lyx::Length::UNIT LengthCombo::currentLengthItem() const
{
	QString const val = itemData(currentIndex()).toString();
	return lyx::unitFromString(lyx::fromqstr(val));
}


void LengthCombo::hasActivated(int)
{
  // emit signal
	selectionChanged(currentLengthItem());
}


void LengthCombo::setCurrentItem(lyx::Length::UNIT unit)
{
	setCurrentItem(lyx::toqstr(lyx::stringFromUnit(unit)));
}


void LengthCombo::setCurrentItem(QString const & item)
{
	int num = QComboBox::count();
	for (int i = 0; i < num; i++) {
		if (QComboBox::itemData(i).toString() == item) {
			QComboBox::setCurrentIndex(i);
			break;
		}
	}
}


void LengthCombo::setCurrentItem(int item)
{
	QComboBox::setCurrentIndex(item);
}


void LengthCombo::setEnabled(bool b)
{
	QComboBox::setEnabled(b);
}


void LengthCombo::noPercents()
{
	int num = QComboBox::count();
	for (int i = 0; i < num; i++) {
		if (QComboBox::itemData(i).toString().contains('%')) {
			QComboBox::removeItem(i);
			--i;
			--num;
		}
	}
}


void LengthCombo::removeFontDependent()
{
	removeUnit(Length::EM);
	removeUnit(Length::EX);
	removeUnit(Length::MU);
}


void LengthCombo::removeUnit(lyx::Length::UNIT unit)
{
	QString const val = lyx::toqstr(lyx::stringFromUnit(unit));
	int num = QComboBox::count();
	for (int i = 0; i < num; i++) {
		if (QComboBox::itemData(i).toString() == val) {
			QComboBox::removeItem(i);
			break;
		}
	}
}


void LengthCombo::addUnit(lyx::Length::UNIT unit)
{
	QString const val = lyx::toqstr(lyx::stringFromUnit(unit));
	int num = QComboBox::count();
	for (int i = 0; i < num; i++) {
		if (QComboBox::itemData(i).toString() == val) {
			// already there, nothing to do
			return;
		}
	}
	insertItem(int(unit), lyx::qt_(lyx::unit_name_gui[int(unit)]),
		   lyx::toqstr(lyx::unit_name[int(unit)]));
}

} // namespace frontend
} // namespace lyx

#include "moc_LengthCombo.cpp"
