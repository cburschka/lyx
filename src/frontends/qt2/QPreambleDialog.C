/**
 * \file QPreambleDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <fstream>

#include "support/lyxlib.h"
#include "support/forkedcall.h"
#include "support/filetools.h"
#include "gettext.h"
#include "LyXView.h"
#include "ControlPreamble.h"

#include "QPreamble.h"
#include "QPreambleDialog.h"

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qinputdialog.h>

using std::getline;

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
			_("Enter editor program"), _("Editor"), QLineEdit::Normal,
			lastentry.c_str()).latin1();
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

	Forkedcall call;

	// FIXME: make async
	if (call.startscript(Forkedcall::Wait, editor)) {
		lyx::unlink(filename);
		return;
	}

	std::ifstream in(filename.c_str());

	if (!in) {
		lyx::unlink(filename);
		return;
	}

	string newtext;
	string line;

	while (getline(in, line)) {
		newtext += line + "\n";
	}

	in.close();
	lyx::unlink(filename);
	preambleLE->setText(newtext.c_str());
}
