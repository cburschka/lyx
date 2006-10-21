/**
 * \file lengthcombo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lengthcombo.h"

#include "lengthcommon.h"


LengthCombo::LengthCombo(QWidget * parent)
	: QComboBox(parent)
{
	for (int i = 0; i < lyx::num_units; i++)
		addItem(lyx::unit_name_gui[i]);

	connect(this, SIGNAL(activated(int)),
		this, SLOT(has_activated(int)));
}


lyx::LyXLength::UNIT LengthCombo::currentLengthItem() const
{
	return static_cast<lyx::LyXLength::UNIT>(currentIndex());
}


void LengthCombo::has_activated(int)
{
  // emit signal
	selectionChanged(currentLengthItem());
}


void LengthCombo::setCurrentItem(lyx::LyXLength::UNIT unit)
{
	QComboBox::setCurrentIndex(int(unit));
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
		if (QComboBox::itemText(i).contains('%') > 0) {
			QComboBox::removeItem(i);
			--i;
			--num;
		}
	}
}

#include "lengthcombo_moc.cpp"
