/**
 * \file citationdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>

#include "FormCitation.h"
#include "citationdlg.h"

#include <qtooltip.h>

CitationDialog::CitationDialog(FormCitation * f, QWidget * p, char const * name)
	: CitationDialogData(p, name), form_(f)
{
	setCaption(name);

	// tooltips

	QToolTip::add(button_add, _("Add reference to current citation"));
	QToolTip::add(button_remove, _("Remove reference from current citation"));
	QToolTip::add(button_up, _("Move reference before"));
	QToolTip::add(button_down, _("Move reference after"));
	QToolTip::add(line_after, _("Text to add after references"));
	QToolTip::add(line_details, _("Reference details"));
	QToolTip::add(button_search, _("Search through references"));
	QToolTip::add(list_available, _("Available references"));
	QToolTip::add(list_chosen, _("Current chosen references"));

	line_details->setMinimumHeight(30);
 
	setMinimumSize(200, 65);
}


void CitationDialog::clicked_search()
{
	// FIXME 
}

 
void CitationDialog::closeEvent(QCloseEvent * e)
{
	form_->CancelButton();
	e->accept();
}
