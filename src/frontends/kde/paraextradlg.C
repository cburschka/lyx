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

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
#endif

ParaExtraDialog::ParaExtraDialog (QWidget * parent, char const * name) 
	: ParaExtraDialogData(parent, name)
{
	type->insertItem(_("Normal"));
	type->insertItem(_("Indented paragraph"));
	type->insertItem(_("Minipage"));
	type->insertItem(_("Wrap text around floats (floatflt)"));
	setSizeHint(type);
	
	widthvalueunits->insertItem(_("Centimetres"));
	widthvalueunits->insertItem(_("Inches"));
	widthvalueunits->insertItem(_("Points (1/72.27 inch)"));
	widthvalueunits->insertItem(_("Millimetres"));
	widthvalueunits->insertItem(_("Picas"));
	widthvalueunits->insertItem(_("ex units"));
	widthvalueunits->insertItem(_("em units"));
	widthvalueunits->insertItem(_("Scaled points (1/65536 pt)"));
	widthvalueunits->insertItem(_("Big/PS points (1/72 inch)"));
	widthvalueunits->insertItem(_("Didot points"));
	widthvalueunits->insertItem(_("Cicero points"));
	widthvalueunits->insertItem(_("Percent of column"));
	setSizeHint(widthvalueunits);
	
	QToolTip::add(hfillbetween, _("FIXME please !"));
	QToolTip::add(startnewminipage, _("FIXME please !"));
}


ParaExtraDialog::~ParaExtraDialog()
{
}


void ParaExtraDialog::typeHighlighted(int val)
{
	specialalignment->setEnabled(val == 2);
	top->setEnabled(val == 2);
	middle->setEnabled(val == 2);
	bottom->setEnabled(val == 2);
	widthvalue->setEnabled(val != 0);
	widthvalueunits->setEnabled(val != 0);
	hfillbetween->setEnabled(val == 2); 
	startnewminipage->setEnabled(val == 2); 
}
