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

	abovepage = new ParaAboveDialogData(this, "abovepage");
	belowpage = new ParaBelowDialogData(this, "belowpage");

	// FIXME: free punctuation to whoever can make the value
	// boxes not be overly large
	spacetab->addTabPage(abovepage, _("Spacing Above"));
	spacetab->addTabPage(belowpage, _("Spacing Below"));

	connect(abovepage->spaceabove, SIGNAL(highlighted(int)), this, SLOT(spaceaboveHighlighted(int)));
	connect(belowpage->spacebelow, SIGNAL(highlighted(int)), this, SLOT(spacebelowHighlighted(int)));

	abovepage->spaceabove->insertItem(_("None"));
	abovepage->spaceabove->insertItem(_("Defskip"));
	abovepage->spaceabove->insertItem(_("Small skip"));
	abovepage->spaceabove->insertItem(_("Medium skip"));
	abovepage->spaceabove->insertItem(_("Big skip"));
	abovepage->spaceabove->insertItem(_("VFill"));
	abovepage->spaceabove->insertItem(_("Length"));
	belowpage->spacebelow->insertItem(_("None"));
	belowpage->spacebelow->insertItem(_("Defskip"));
	belowpage->spacebelow->insertItem(_("Small skip"));
	belowpage->spacebelow->insertItem(_("Medium skip"));
	belowpage->spacebelow->insertItem(_("Big skip"));
	belowpage->spacebelow->insertItem(_("VFill"));
	belowpage->spacebelow->insertItem(_("Length"));
 
	createUnits(abovepage->spaceabovevalueunits);
	createUnits(abovepage->spaceaboveplusunits);
	createUnits(abovepage->spaceaboveminusunits);
	createUnits(belowpage->spacebelowvalueunits);
	createUnits(belowpage->spacebelowplusunits);
	createUnits(belowpage->spacebelowminusunits);
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
}

ParaGeneralDialog::~ParaGeneralDialog()
{
}

void ParaGeneralDialog::spaceaboveHighlighted(int val)
{
	abovepage->spaceabovevalue->setEnabled(val == 6);
	abovepage->spaceabovevalueunits->setEnabled(val == 6);
	abovepage->spaceaboveplus->setEnabled(val == 6);
	abovepage->spaceaboveplusunits->setEnabled(val == 6);
	abovepage->spaceaboveminus->setEnabled(val == 6);
	abovepage->spaceaboveminusunits->setEnabled(val == 6);
}

void ParaGeneralDialog::spacebelowHighlighted(int val)
{
	belowpage->spacebelowvalue->setEnabled(val == 6);
	belowpage->spacebelowvalueunits->setEnabled(val == 6);
	belowpage->spacebelowplus->setEnabled(val == 6);
	belowpage->spacebelowplusunits->setEnabled(val == 6);
	belowpage->spacebelowminus->setEnabled(val == 6);
	belowpage->spacebelowminusunits->setEnabled(val == 6);
}
