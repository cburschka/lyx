/**
 * \file parageneraldlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "parageneraldlg.h"

#include "dlg/helpers.h"

#include <gettext.h>

#include <qtooltip.h>

using kde_helpers::setSizeHint;

ParaGeneralDialog::ParaGeneralDialog (QWidget * parent, char const * name)
	: ParaGeneralDialogData(parent, name)
{

	abovepage = new ParaAboveDialogData(this, "abovepage");
	belowpage = new ParaBelowDialogData(this, "belowpage");

	spacetab->addTabPage(abovepage, _("&Spacing Above"));
	spacetab->addTabPage(belowpage, _("Spacing &Below"));

	connect(abovepage->spaceabove, SIGNAL(highlighted(int)), this, SLOT(spaceaboveHighlighted(int)));
	connect(belowpage->spacebelow, SIGNAL(highlighted(int)), this, SLOT(spacebelowHighlighted(int)));

	abovepage->spaceabove->insertItem(_("None"));
	abovepage->spaceabove->insertItem(_("Defskip"));
	abovepage->spaceabove->insertItem(_("Small skip"));
	abovepage->spaceabove->insertItem(_("Medium skip"));
	abovepage->spaceabove->insertItem(_("Big skip"));
	abovepage->spaceabove->insertItem(_("VFill"));
	abovepage->spaceabove->insertItem(_("Custom"));
	setSizeHint(abovepage->spaceabove); 

	belowpage->spacebelow->insertItem(_("None"));
	belowpage->spacebelow->insertItem(_("Defskip"));
	belowpage->spacebelow->insertItem(_("Small skip"));
	belowpage->spacebelow->insertItem(_("Medium skip"));
	belowpage->spacebelow->insertItem(_("Big skip"));
	belowpage->spacebelow->insertItem(_("VFill"));
	belowpage->spacebelow->insertItem(_("Custom"));
	setSizeHint(belowpage->spacebelow); 
	
	justification->insertItem(_("Block"));
	justification->insertItem(_("Centered"));
	justification->insertItem(_("Left"));
	justification->insertItem(_("Right"));
	setSizeHint(justification); 
 
	QToolTip::add(justification, _("Alignment of current paragraph")); 
	QToolTip::add(noindent, _("No indent on first line of paragraph")); 
	QToolTip::add(labelwidth, _("FIXME please !")); 
	
	QToolTip::add(abovepage->pagebreakabove, _("New page above this paragraph"));
	QToolTip::add(abovepage->keepabove, _("Don't hug margin if at top of page"));
	QToolTip::add(abovepage->spaceabovevalue, _("Size of extra space above paragraph"));
	QToolTip::add(abovepage->spaceaboveplus, _("Maximum extra space that can be added"));
	QToolTip::add(abovepage->spaceaboveminus, _("Minimum space required"));
 
	QToolTip::add(belowpage->pagebreakbelow, _("New page below this paragraph"));
	QToolTip::add(belowpage->keepbelow, _("Don't hug margin if at bottom of page"));
	QToolTip::add(belowpage->spacebelowvalue, _("Size of extra space below paragraph"));
	QToolTip::add(belowpage->spacebelowplus, _("Maximum extra space that can be added"));
	QToolTip::add(belowpage->spacebelowminus, _("Minimum space required"));
}


ParaGeneralDialog::~ParaGeneralDialog()
{
}


void ParaGeneralDialog::spaceaboveHighlighted(int val)
{
	abovepage->spaceabovevalue->setEnabled(val == 6);
	abovepage->spaceaboveplus->setEnabled(val == 6);
	abovepage->spaceaboveminus->setEnabled(val == 6);
}


void ParaGeneralDialog::spacebelowHighlighted(int val)
{
	belowpage->spacebelowvalue->setEnabled(val == 6);
	belowpage->spacebelowplus->setEnabled(val == 6);
	belowpage->spacebelowminus->setEnabled(val == 6);
}
