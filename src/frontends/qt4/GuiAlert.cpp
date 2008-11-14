/**
 * \file qt4/GuiAlert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "alert.h"

#include "frontends/Application.h"

#include "qt_helpers.h"
#include "LyX.h" // for lyx::use_gui
#include "ui_AskForTextUi.h"
#include "support/gettext.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/lstrings.h"

#include <QApplication>
#include <QMessageBox>
#include <QLineEdit>
#include <QInputDialog>

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


namespace Alert {

int prompt(docstring const & title0, docstring const & question,
		  int default_button, int cancel_button,
		  docstring const & b1, docstring const & b2, docstring const & b3)
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
		}
		if (!use_gui)
			return default_button;
	}

	docstring const title = bformat(_("LyX: %1$s"), title0);

	// For some reason, sometimes Qt uses an hourglass or watch cursor when
	// displaying the alert. Hence, we ask for the standard cursor shape.
	// This call has no effect if the cursor has not been overridden.
	qApp->changeOverrideCursor(Qt::ArrowCursor);

	// FIXME replace that with guiApp->currentView()
	//LYXERR0("FOCUS: " << qApp->focusWidget());
	int res = QMessageBox::information(qApp->focusWidget(),
					   toqstr(title),
					   toqstr(formatted(question)),
					   toqstr(b1),
					   toqstr(b2),
					   b3.empty() ? QString::null : toqstr(b3),
					   default_button, cancel_button);

	// Qt bug: can return -1 on cancel or WM close, despite the docs.
	if (res == -1)
		res = cancel_button;
	return res;
}


void warning(docstring const & title0, docstring const & message)
{
	lyxerr << "Warning: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		int argc = 1;
		char * argv[1];
		QApplication app(argc, argv);
		QMessageBox::warning(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	QMessageBox::warning(qApp->focusWidget(),
			     toqstr(title),
			     toqstr(formatted(message)));
}


int argc = 1;
char * argv[1];

void error(docstring const & title0, docstring const & message)
{
	lyxerr << "Error: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);
	if (theApp() == 0) {
		QApplication app(argc, argv);
		QMessageBox::critical(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	QMessageBox::critical(qApp->focusWidget(),
			      toqstr(title),
			      toqstr(formatted(message)));
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
	QMessageBox::information(qApp->focusWidget(),
				 toqstr(title),
				 toqstr(formatted(message)));
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
