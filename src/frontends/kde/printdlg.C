/**
 * \file printdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>
#include <cstring>

#include "support/lstrings.h"
#include "support/filetools.h"
 
#include "printdlg.h"
#include "dlg/printdlgdata.h"

#include <qtooltip.h>
#include <kfiledialog.h>

#define Inherited PrintDialogData

PrintDialog::PrintDialog(FormPrint * f, QWidget * parent, char const * name)
	: PrintDialogData(parent, name), form_(f)
{
	setCaption(name); 
	QToolTip::add(radio_allpages, _("Print every page"));
	QToolTip::add(radio_oddpages, _("Print odd-numbered pages only"));
	QToolTip::add(radio_evenpages, _("Print even-numbered pages only"));
	QToolTip::add(line_from, _("Print from page number")); 
	QToolTip::add(line_to, _("Print to page number")); 
	QToolTip::add(check_reverse, _("Print in reverse order (last page first)"));
	QToolTip::add(line_count, _("Number of copies to print"));
	QToolTip::add(check_sort, _("Collate multiple copies"));
	QToolTip::add(line_printername, _("Printer name"));
	QToolTip::add(line_filename, _("Output filename (PostScript)"));
	QToolTip::add(button_browse, _("Select output filename"));
}


PrintDialog::~PrintDialog()
{
}


void PrintDialog::clickedBrowse()
{
	// FIXME: move into utility class later, accept URLs etc.

	KFileDialog filedlg(OnlyPath(line_filename->text()).c_str(), "*.ps|PostScript Files (*.ps)", this, 0, true, false);

	if (filedlg.exec() == QDialog::Accepted)
		line_filename->setText(filedlg.selectedFile().data());
}


void PrintDialog::changedCount(char const * text)
{
	if (compare("", text) && strToInt(text) == 0) {
		line_count->setText("1");
		return;
	}

	check_sort->setEnabled(strToInt(text) > 1);
}


void PrintDialog::closeEvent(QCloseEvent * e)
{
	form_->CancelButton();
	e->accept();
}
