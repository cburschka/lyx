/*
 * formprintdialog.C
 * (C) 2000, LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include "formprintdialogdata.h"
#include "formprintdialog.h"
#include "support/lstrings.h"

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
	// FIXME: open a file dialog here
}
