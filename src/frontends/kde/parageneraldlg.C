/**********************************************************************

	--- Qt Architect generated file ---

	File: parageneraldlg.C
	Last generated: Sat Oct 14 00:27:47 2000

 *********************************************************************/

#include "parageneraldlg.h"

#include <gettext.h>

#define Inherited ParaGeneralDialogData

ParaGeneralDialog::ParaGeneralDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{

	spaceabove->insertItem(_("None"));
	spaceabove->insertItem(_("Defskip"));
	spaceabove->insertItem(_("Small skip"));
	spaceabove->insertItem(_("Medium skip"));
	spaceabove->insertItem(_("Big skip"));
	spaceabove->insertItem(_("VFill"));
	spaceabove->insertItem(_("Length"));
	spacebelow->insertItem(_("None"));
	spacebelow->insertItem(_("Defskip"));
	spacebelow->insertItem(_("Small skip"));
	spacebelow->insertItem(_("Medium skip"));
	spacebelow->insertItem(_("Big skip"));
	spacebelow->insertItem(_("VFill"));
	spacebelow->insertItem(_("Length"));
 
	createUnits(spaceabovevalueunits);
	createUnits(spaceaboveplusunits);
	createUnits(spaceaboveminusunits);
	createUnits(spacebelowvalueunits);
	createUnits(spacebelowplusunits);
	createUnits(spacebelowminusunits);
}

void ParaGeneralDialog::createUnits(QComboBox *box)
{
	// if you change this, remember to change ParaDialog::get/setUnits()
	box->insertItem(_("Centimetres"));
	box->insertItem(_("Inches"));
	box->insertItem(_("Points (1/72.27 inch)"));
	box->insertItem(_("Millimetres"));
	box->insertItem(_("Picas"));
	box->insertItem(_("ex units"));
	box->insertItem(_("em units"));
	box->insertItem(_("Scaled points (1/65536 pt)"));
	box->insertItem(_("Big/PS points (1/72 inch)"));
	box->insertItem(_("Didot points"));
	box->insertItem(_("Cicero points"));
	box->insertItem(_("Math units"));
}

ParaGeneralDialog::~ParaGeneralDialog()
{
}

void ParaGeneralDialog::spaceaboveHighlighted(int val)
{
	spaceabovevalue->setEnabled(val == 6);
	spaceabovevalueunits->setEnabled(val == 6);
	spaceaboveplus->setEnabled(val == 6);
	spaceaboveplusunits->setEnabled(val == 6);
	spaceaboveminus->setEnabled(val == 6);
	spaceaboveminusunits->setEnabled(val == 6);
}

void ParaGeneralDialog::spaceabovevalueChanged(const char *text)
{
	// FIXME: the problem here is not validation, but what to do
	// if it is not a valid double value ...
}

void ParaGeneralDialog::spaceaboveplusChanged(const char *text)
{
	// FIXME
}

void ParaGeneralDialog::spaceaboveminusChanged(const char *text)
{
	// FIXME
}

void ParaGeneralDialog::spacebelowHighlighted(int val)
{
	spacebelowvalue->setEnabled(val == 6);
	spacebelowvalueunits->setEnabled(val == 6);
	spacebelowplus->setEnabled(val == 6);
	spacebelowplusunits->setEnabled(val == 6);
	spacebelowminus->setEnabled(val == 6);
	spacebelowminusunits->setEnabled(val == 6);
}

void ParaGeneralDialog::spacebelowvalueChanged(const char *text)
{
	// FIXME
}

void ParaGeneralDialog::spacebelowplusChanged(const char *text)
{
	// FIXME
}

void ParaGeneralDialog::spacebelowminusChanged(const char *text)
{
	// FIXME
}
