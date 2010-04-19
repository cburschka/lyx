/**
 * \file qt4/GuiAlert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "alert.h"


#include "frontends/Application.h"

#include "qt_helpers.h"
#include "LyX.h" // for lyx::use_gui
#include "support/gettext.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/lstrings.h"
#include "support/ProgressInterface.h"

#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QInputDialog>
#include <QPushButton>
#include <QSettings>

#include <iomanip>
#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {




static docstring const formatted(docstring const & text)
{
	const int w = 80;
	docstring sout;

	if (text.empty())
		return sout;

	size_t curpos = 0;
	docstring line;

	while (true) {
		size_t const nxtpos1 = text.find(' ',  curpos);
		size_t const nxtpos2 = text.find('\n', curpos);
		size_t const nxtpos = min(nxtpos1, nxtpos2);

		docstring const word =
			nxtpos == docstring::npos ?
			text.substr(curpos) :
			text.substr(curpos, nxtpos - curpos);

		bool const newline = (nxtpos2 != docstring::npos &&
				      nxtpos2 < nxtpos1);

		docstring const line_plus_word =
			line.empty() ? word : line + char_type(' ') + word;

		// FIXME: make w be size_t
		if (int(line_plus_word.length()) >= w) {
			sout += line + char_type('\n');
			if (newline) {
				sout += word + char_type('\n');
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + char_type('\n');
			line.erase();

		} else {
			if (!line.empty())
				line += char_type(' ');
			line += word;
		}

		if (nxtpos == docstring::npos) {
			if (!line.empty())
				sout += line;
			break;
		}

		curpos = nxtpos + 1;
	}

	return sout;
}


void noAppDialog(QString const & title, QString const & msg, QMessageBox::Icon mode)
{
	int argc = 1;
	const char *argv[] = { "lyx", 0 };

	QApplication app(argc, (char**)argv);
	switch (mode)
	{
		case QMessageBox::Information: QMessageBox::information(0, title, msg); break;
		case QMessageBox::Warning: QMessageBox::warning(0, title, msg); break;
		case QMessageBox::Critical: QMessageBox::critical(0, title, msg); break;
		default: break;
	}
}


namespace Alert {

int prompt(docstring const & title0, docstring const & question,
		  int default_button, int cancel_button,
		  docstring const & b1, docstring const & b2,
		  docstring const & b3, docstring const & b4)
{
	//lyxerr << "PROMPT" << title0 << "FOCUS: " << qApp->focusWidget() << endl;
	if (!use_gui || lyxerr.debugging()) {
		lyxerr << title0 << '\n'
		       << "----------------------------------------\n"
		       << question << endl;

		lyxerr << "Assuming answer is ";
		switch (default_button) {
		case 0: lyxerr << b1 << endl;
		case 1: lyxerr << b2 << endl;
		case 2: lyxerr << b3 << endl;
		case 3: lyxerr << b4 << endl;
		}
		if (!use_gui)
			return default_button;
	}

	docstring const title = bformat(_("LyX: %1$s"), title0);

	// For some reason, sometimes Qt uses a hourglass or watch cursor when
	// displaying the alert. Hence, we ask for the standard cursor shape.
	qApp->setOverrideCursor(Qt::ArrowCursor);

	// FIXME replace that with guiApp->currentView()
	//LYXERR0("FOCUS: " << qApp->focusWidget());
	QPushButton * b[4] = { 0, 0, 0, 0 };
	QMessageBox msg_box(QMessageBox::Information,
			toqstr(title), toqstr(formatted(question)),
			QMessageBox::NoButton, qApp->focusWidget());
	b[0] = msg_box.addButton(b1.empty() ? "OK" : toqstr(b1),
					QMessageBox::ActionRole);
	if (!b2.empty())
		b[1] = msg_box.addButton(toqstr(b2), QMessageBox::ActionRole);
	if (!b3.empty())
		b[2] = msg_box.addButton(toqstr(b3), QMessageBox::ActionRole);
	if (!b4.empty())
		b[3] = msg_box.addButton(toqstr(b4), QMessageBox::ActionRole);
	msg_box.setDefaultButton(b[default_button]);
	msg_box.setEscapeButton(static_cast<QAbstractButton *>(b[cancel_button]));
	int res = msg_box.exec();

	qApp->restoreOverrideCursor();

	// Qt bug: can return -1 on cancel or WM close, despite the docs.
	if (res == -1)
		res = cancel_button;
	return res;
}


void warning(docstring const & title0, docstring const & message,
	     bool const & askshowagain)
{
	lyxerr << "Warning: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(formatted(message)), QMessageBox::Warning);
		return;
	}

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	if (!askshowagain) {
		ProgressInterface::instance()->warning(
				toqstr(title),
				toqstr(formatted(message)));
	} else {
		ProgressInterface::instance()->toggleWarning(
				toqstr(title),
				toqstr(message),
				toqstr(formatted(message)));
	}

	qApp->restoreOverrideCursor();
}


void error(docstring const & title0, docstring const & message)
{
	lyxerr << "Error: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(formatted(message)), QMessageBox::Critical);
		return;
	}

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	ProgressInterface::instance()->error(
		toqstr(title),
		toqstr(formatted(message)));

	qApp->restoreOverrideCursor();
}


void information(docstring const & title0, docstring const & message)
{
	if (!use_gui || lyxerr.debugging())
		lyxerr << title0 << '\n'
		       << "----------------------------------------\n"
		       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(formatted(message)), QMessageBox::Information);
		return;
	}

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	ProgressInterface::instance()->information(
		toqstr(title),
		toqstr(formatted(message)));

	qApp->restoreOverrideCursor();
}


bool askForText(docstring & response, docstring const & msg,
	docstring const & dflt)
{
	if (!use_gui || lyxerr.debugging()) {
		lyxerr << "----------------------------------------\n"
		       << msg << '\n'
		       << "Assuming answer is " << dflt << '\n'
		       << "----------------------------------------" << endl;
		if (!use_gui) {
			response = dflt;
			return true;
		}
	}

	docstring const title = bformat(_("LyX: %1$s"), msg);

	bool ok;
	QString text = QInputDialog::getText(qApp->focusWidget(),
		toqstr(title),
		toqstr(char_type('&') + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (ok) {
		response = qstring_to_ucs4(text);
		return true;
	}
	response.clear();
	return false;
}


} // namespace Alert
} // namespace frontend
} // namespace lyx
