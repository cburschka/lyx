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


LengthCombo::LengthCombo(QWidget * parent, char * name)
	: QComboBox(parent, name)
{
	for (int i=0; i < num_units; i++)
		insertItem(unit_name_gui[i]);

	connect(this, SIGNAL(activated(int)),
		this, SLOT(has_activated(int)));
}


LyXLength::UNIT LengthCombo::currentLengthItem() const
{
	return static_cast<LyXLength::UNIT>(currentItem());
}


void LengthCombo::has_activated(int)
{
	emit selectionChanged(currentLengthItem());
}


void LengthCombo::setCurrentItem(LyXLength::UNIT unit)
{
	QComboBox::setCurrentItem(int(unit));
}


void LengthCombo::setCurrentItem(int item)
{
	QComboBox::setCurrentItem(item);
}


void LengthCombo::setEnabled(bool b)
{
	QComboBox::setEnabled(b);
}


void LengthCombo::noPercents()
{
	int num = QComboBox::count();
	for (int i = 0; i < num; i++) {
		if (QComboBox::text(i).contains("%") > 0) {
			QComboBox::removeItem(i);
			i -= 1;
			num -= 1;
		}
	}
}
