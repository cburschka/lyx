/**
 * \file lengthcombo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lengthcombo.h"
#include <qwhatsthis.h>

#include "lengthcommon.h"
#include "gettext.h"


LengthCombo::LengthCombo(QWidget * parent, char * name)
	: QComboBox(parent, name)
{
	for (int i=0; i < num_units; i++)
		insertItem(unit_name_gui[i]);

	connect(this, SIGNAL(activated(int)),
		this, SLOT(has_activated(int)));

	QWhatsThis::add(this, _("Choose one of the units or relative lengths"));
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


void LengthCombo::setEnabled(bool b)
{
	QComboBox::setEnabled(b);
}
