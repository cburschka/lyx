/*
 * formprintdialog.C
 * (C) 2000, LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include "formprintdialogdata.h"
#include "formprintdialog.h"
#include "support/lstrings.h"
#include "support/filetools.h"

#include <kfiledialog.h>
#include <gettext.h>
#include <cstring>

#define Inherited FormPrintDialogData

FormPrintDialog::FormPrintDialog(FormPrint *f, QWidget* parent, const char * name)
	: FormPrintDialogData(parent, name), form_(f)
{
	setCaption(name); 
}

FormPrintDialog::~FormPrintDialog()
{
}

void FormPrintDialog::clickedBrowse()
{
	// FIXME: move into utility class later, accept URLs etc.

	KFileDialog filedlg(OnlyPath(filename->text()).c_str(), "*.ps|PostScript Files (*.ps)", this, 0, true, false);

	if (filedlg.exec() == QDialog::Accepted)
		filename->setText(filedlg.selectedFile().data());
}
