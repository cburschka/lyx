/**
 * \file QPreambleDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <fstream>

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qinputdialog.h>

#include "QPreambleDialog.h"
#include "Dialogs.h"
#include "QPreamble.h"

#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/filetools.h"
#include "gettext.h" 
 
#include "QtLyXView.h"
#include "ControlPreamble.h" 

QPreambleDialog::QPreambleDialog(QPreamble * form)
	: QPreambleDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QPreambleDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QPreambleDialog::change_adaptor()
{
	form_->changed();
}

 
void QPreambleDialog::editClicked()
{
	// find an editor
	string editor = GetEnv("EDITOR");
	if (editor.empty()) {
		static string lastentry = "";
		editor = QInputDialog::getText(
			_("Enter editor program"), _("Editor"), lastentry.c_str()).latin1();
		if (editor.empty())
			return;
		lastentry = editor; 
	}

	string const text(preambleLE->text().latin1());
	string const filename(lyx::tempName("", "preamble"));
	std::ofstream file(filename.c_str());

	// FIXME ?
	if (!file)
		return;

	file << text;

	file.close();

	editor += " " + filename;
 
	// FIXME: synchronous, ugh. Make async when moved to controllers ?
	Systemcalls sys(Systemcalls::Wait, editor);

	std::ifstream in(filename.c_str());

	if (!in)
		return;

	string newtext;
	string line;
 
	while (getline(in, line)) {
		newtext += line + "\n"; 
	}

	in.close();
	lyx::unlink(filename);
	preambleLE->setText(newtext.c_str());
}
