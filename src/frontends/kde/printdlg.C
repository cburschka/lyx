/*
 * printdlg.C
 * (C) 2000, LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include "printdlgdata.h"
#include "printdlg.h"
#include "support/lstrings.h"
#include "support/filetools.h"

#include <qtooltip.h>
#include <kfiledialog.h>

#include <gettext.h>
#include <cstring>

#define Inherited PrintDialogData

PrintDialog::PrintDialog(FormPrint *f, QWidget* parent, const char * name)
	: PrintDialogData(parent, name), form_(f)
{
	setCaption(name); 
	QToolTip::add(allpages, _("Print every page"));
	QToolTip::add(oddpages, _("Print odd-numbered pages only"));
	QToolTip::add(evenpages, _("Print even-numbered pages only"));
	QToolTip::add(from, _("Print from page number")); 
	QToolTip::add(to, _("Print to page number")); 
	QToolTip::add(reverse, _("Print in reverse order (last page first)"));
	QToolTip::add(count, _("Number of copies to print"));
	QToolTip::add(sort, _("Collate multiple copies"));
	QToolTip::add(printername, _("Printer name"));
	QToolTip::add(filename, _("Output filename"));
	QToolTip::add(browse, _("Select output filename"));
}

PrintDialog::~PrintDialog()
{
}

void PrintDialog::clickedBrowse()
{
	// FIXME: move into utility class later, accept URLs etc.

	KFileDialog filedlg(OnlyPath(filename->text()).c_str(), "*.ps|PostScript Files (*.ps)", this, 0, true, false);

	if (filedlg.exec() == QDialog::Accepted)
		filename->setText(filedlg.selectedFile().data());
}

void PrintDialog::changedCount(const char *text)
{
	if (strcmp("",text) && strToInt(text) == 0) {
		count->setText("1");
		return;
	}

	sort->setEnabled(strToInt(text) > 1);
}
