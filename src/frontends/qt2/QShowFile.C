/**
 * \file QShowFile.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QShowFileDialog.h"
#include "ControlShowFile.h"
#include "QShowFile.h"
#include "Qt2BC.h"
#include "gettext.h"

#include <qtextview.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlShowFile, Qt2DB<QShowFileDialog> > base_class;

QShowFile::QShowFile(ControlShowFile & c)
	: base_class(c, _("ShowFile"))
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
