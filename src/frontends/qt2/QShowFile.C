/**
 * \file QShowFile.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlShowFile.h"
#include "gettext.h"

#include "QShowFile.h"
#include "QShowFileDialog.h"
#include "Qt2BC.h"
 
#include <qtextview.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlShowFile, Qt2DB<QShowFileDialog> > base_class;

QShowFile::QShowFile()
	: base_class(_("ShowFile"))
{
}


void QShowFile::build_dialog()
{
	dialog_.reset(new QShowFileDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QShowFile::update_contents()
{
	dialog_->setName(controller().getFileName().c_str());

	string contents = controller().getFileContents();
	if (contents.empty()) {
		contents = "Error -> Cannot load file!";
	}

	dialog_->text->setText(contents.c_str());
}
