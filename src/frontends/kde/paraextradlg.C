/**
 * \file paraextradlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "paraextradlg.h"

#include "dlg/helpers.h"

#include <qtooltip.h>

#include <gettext.h>

using kde_helpers::setSizeHint;

ParaExtraDialog::ParaExtraDialog (QWidget * parent, char const * name) 
	: ParaExtraDialogData(parent, name)
{
	combo_type->insertItem(_("Normal"));
	combo_type->insertItem(_("Indented paragraph"));
	combo_type->insertItem(_("Minipage"));
	combo_type->insertItem(_("Wrap text around floats (floatflt)"));
	setSizeHint(combo_type);
	
	combo_widthvalueunits->insertItem(_("Centimetres"));
	combo_widthvalueunits->insertItem(_("Inches"));
	combo_widthvalueunits->insertItem(_("Points (1/72.27 inch)"));
	combo_widthvalueunits->insertItem(_("Millimetres"));
	combo_widthvalueunits->insertItem(_("Picas"));
	combo_widthvalueunits->insertItem(_("ex units"));
	combo_widthvalueunits->insertItem(_("em units"));
	combo_widthvalueunits->insertItem(_("Scaled points (1/65536 pt)"));
	combo_widthvalueunits->insertItem(_("Big/PS points (1/72 inch)"));
	combo_widthvalueunits->insertItem(_("Didot points"));
	combo_widthvalueunits->insertItem(_("Cicero points"));
	combo_widthvalueunits->insertItem(_("Percent of column"));
	setSizeHint(combo_widthvalueunits);
	
	QToolTip::add(check_hfillbetween, _("FIXME please !"));
	QToolTip::add(check_startnewminipage, _("FIXME please !"));
}


ParaExtraDialog::~ParaExtraDialog()
{
}


void ParaExtraDialog::typeHighlighted(int val)
{
	specialalignment->setEnabled(val == 2);
	radio_top->setEnabled(val == 2);
	radio_middle->setEnabled(val == 2);
	radio_bottom->setEnabled(val == 2);
	line_widthvalue->setEnabled(val != 0);
	combo_widthvalueunits->setEnabled(val != 0);
	check_hfillbetween->setEnabled(val == 2); 
	check_startnewminipage->setEnabled(val == 2); 
}
