/**
 * \file lengthcombo.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "gettext.h"
 
#include "lengthcombo.h"
 
#include <qwhatsthis.h> 

LengthCombo::LengthCombo(QWidget * parent, char * name)
	: QComboBox(parent, name)
{
	insertItem(_("Centimetres"));
	insertItem(_("Inches"));
	insertItem(_("Points"));
	insertItem(_("Millimetres"));
	insertItem(_("Picas"));
	insertItem(_("ex units"));
	insertItem(_("em units"));
	insertItem(_("Scaled points"));
	insertItem(_("Big/PS points"));
	insertItem(_("Didot points"));
	insertItem(_("Cicero points"));
	insertItem(_("Mathematical units"));
	insertItem(_("Percent of page"));
	insertItem(_("Percent of column"));
	insertItem(_("Percent of line width"));

	connect(this, SIGNAL(activated(int)),
		this, SLOT(has_activated(int)));

	QWhatsThis::add(this, _("FIXME - describe the units."));
}


LyXLength::UNIT LengthCombo::currentLengthItem() const
{ 
	LyXLength::UNIT unit;
	int i = currentItem();
	switch (i) {
		default:
		case 0: unit = LyXLength::CM; break;
		case 1: unit = LyXLength::IN; break;
		case 2: unit = LyXLength::PT; break;
		case 3: unit = LyXLength::MM; break;
		case 4: unit = LyXLength::PC; break;
		case 5: unit = LyXLength::EX; break;
		case 6: unit = LyXLength::EM; break;
		case 7: unit = LyXLength::SP; break;
		case 8: unit = LyXLength::BP; break;
		case 9: unit = LyXLength::DD; break;
		case 10: unit = LyXLength::CC; break;
		case 11: unit = LyXLength::MU; break;
		case 12: unit = LyXLength::PP; break;
		case 13: unit = LyXLength::PW; break;
		case 14: unit = LyXLength::PL; break;
		// FIXME: LyXLength::PE ?
	};
	return unit;
} 
 

void LengthCombo::has_activated(int)
{
	emit selectionChanged(currentLengthItem());
}

 
void LengthCombo::setCurrentItem(LyXLength::UNIT unit)
{
	int i;
	switch (unit) {
		default:
		case LyXLength::CM: i = 0; break;
		case LyXLength::IN: i = 1; break;
		case LyXLength::PT: i = 2; break;
		case LyXLength::MM: i = 3; break;
		case LyXLength::PC: i = 4; break;
		case LyXLength::EX: i = 5; break;
		case LyXLength::EM: i = 6; break;
		case LyXLength::SP: i = 7; break;
		case LyXLength::BP: i = 8; break;
		case LyXLength::DD: i = 9; break;
		case LyXLength::CC: i = 10; break;
		case LyXLength::MU: i = 11; break;
		case LyXLength::PP: i = 12; break;
		case LyXLength::PW: i = 13; break;
		case LyXLength::PL: i = 14; break;
		// FIXME: LyXLength::PE ?
	}
	QComboBox::setCurrentItem(i);
}
