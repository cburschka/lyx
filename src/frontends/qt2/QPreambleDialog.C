/**
 * \file QPreambleDialog.C
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

#include "Lsstream.h"
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

#include <fstream>

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
		static string lastentry;
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

	ostringstream newtext;
	newtext << in.rdbuf();

	// close the files before we delete the file
	in.close();

	lyx::unlink(filename);
	preambleLE->setText(newtext.str().c_str());
}
