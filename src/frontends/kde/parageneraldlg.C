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

	tabstack->addTabPage(abovepage, _("&Spacing Above"));
	tabstack->addTabPage(belowpage, _("Spacing &Below"));

	connect(abovepage->combo_spaceabove, SIGNAL(highlighted(int)), this, SLOT(spaceaboveHighlighted(int)));
	connect(belowpage->combo_spacebelow, SIGNAL(highlighted(int)), this, SLOT(spacebelowHighlighted(int)));

	abovepage->combo_spaceabove->insertItem(_("None"));
	abovepage->combo_spaceabove->insertItem(_("Defskip"));
	abovepage->combo_spaceabove->insertItem(_("Small skip"));
	abovepage->combo_spaceabove->insertItem(_("Medium skip"));
	abovepage->combo_spaceabove->insertItem(_("Big skip"));
	abovepage->combo_spaceabove->insertItem(_("VFill"));
	abovepage->combo_spaceabove->insertItem(_("Custom"));
	setSizeHint(abovepage->combo_spaceabove); 

	belowpage->combo_spacebelow->insertItem(_("None"));
	belowpage->combo_spacebelow->insertItem(_("Defskip"));
	belowpage->combo_spacebelow->insertItem(_("Small skip"));
	belowpage->combo_spacebelow->insertItem(_("Medium skip"));
	belowpage->combo_spacebelow->insertItem(_("Big skip"));
	belowpage->combo_spacebelow->insertItem(_("VFill"));
	belowpage->combo_spacebelow->insertItem(_("Custom"));
	setSizeHint(belowpage->combo_spacebelow); 
	
	combo_justification->insertItem(_("Block"));
	combo_justification->insertItem(_("Centered"));
	combo_justification->insertItem(_("Left"));
	combo_justification->insertItem(_("Right"));
	setSizeHint(combo_justification); 
 
	QToolTip::add(combo_justification, _("Alignment of current paragraph")); 
	QToolTip::add(check_noindent, _("No indent on first line of paragraph")); 
	QToolTip::add(line_labelwidth, _("FIXME please !")); 
	
	QToolTip::add(abovepage->check_pagebreakabove, _("New page above this paragraph"));
	QToolTip::add(abovepage->check_keepabove, _("Don't hug margin if at top of page"));
	QToolTip::add(abovepage->length_spaceabove, _("Size of extra space above paragraph"));
	QToolTip::add(abovepage->length_spaceaboveplus, _("Maximum extra space that can be added"));
	QToolTip::add(abovepage->length_spaceaboveminus, _("Minimum space required"));
 
	QToolTip::add(belowpage->check_pagebreakbelow, _("New page below this paragraph"));
	QToolTip::add(belowpage->check_keepbelow, _("Don't hug margin if at bottom of page"));
	QToolTip::add(belowpage->length_spacebelow, _("Size of extra space below paragraph"));
	QToolTip::add(belowpage->length_spacebelowplus, _("Maximum extra space that can be added"));
	QToolTip::add(belowpage->length_spacebelowminus, _("Minimum space required"));
}


ParaGeneralDialog::~ParaGeneralDialog()
{
}


void ParaGeneralDialog::spaceaboveHighlighted(int val)
{
	abovepage->length_spaceabove->setEnabled(val == 6);
	abovepage->length_spaceaboveplus->setEnabled(val == 6);
	abovepage->length_spaceaboveminus->setEnabled(val == 6);
}


void ParaGeneralDialog::spacebelowHighlighted(int val)
{
	belowpage->length_spacebelow->setEnabled(val == 6);
	belowpage->length_spacebelowplus->setEnabled(val == 6);
	belowpage->length_spacebelowminus->setEnabled(val == 6);
}
