/**********************************************************************

	--- Qt Architect generated file ---

	File: paraextradlg.C
	Last generated: Sat Oct 14 00:27:49 2000

 *********************************************************************/

#include "paraextradlg.h"

#include <gettext.h>

#define Inherited ParaExtraDialogData

ParaExtraDialog::ParaExtraDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
	type->insertItem(_("Normal"));
	type->insertItem(_("Indented paragraph"));
	type->insertItem(_("Minipage"));
	type->insertItem(_("Wrap text around floats (floatflt)"));
	
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
}


ParaExtraDialog::~ParaExtraDialog()
{
}

void ParaExtraDialog::typeHighlighted(int val)
{
	specialalignment->setEnabled(val==2);
	top->setEnabled(val==2);
	middle->setEnabled(val==2);
	bottom->setEnabled(val==2);
	widthvalue->setEnabled(val!=0);
	widthvalueunits->setEnabled(val!=0);
	hfillbetween->setEnabled(val==2); 
	startnewminipage->setEnabled(val==2); 
}
