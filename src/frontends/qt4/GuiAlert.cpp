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
#include "InGuiThread.h"

#include "frontends/Application.h"

#include "qt_helpers.h"
#include "LyX.h" // for lyx::use_gui

#include "support/gettext.h"
#include "support/debug.h"
#include "support/docstring.h"
#include "support/lstrings.h"
#include "support/lassert.h"
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


// sync with GuiView.cpp
#define EXPORT_in_THREAD 1


using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


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


int doPrompt(docstring const & title0, docstring const & question,
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
		case 0: lyxerr << b1 << endl; break;
		case 1: lyxerr << b2 << endl; break;
		case 2: lyxerr << b3 << endl; break;
		case 3: lyxerr << b4 << endl;
		}
		if (!use_gui)
			return default_button;
	}

	docstring const title = bformat(_("LyX: %1$s"), title0);

	/// Long operation in progress prevents user from Ok-ing the error dialog
	bool long_op = theApp()->longOperationStarted();
	if (long_op)
		theApp()->stopLongOperation();

	// For some reason, sometimes Qt uses a hourglass or watch cursor when
	// displaying the alert. Hence, we ask for the standard cursor shape.
	qApp->setOverrideCursor(Qt::ArrowCursor);

	// FIXME replace that with guiApp->currentView()
	//LYXERR0("FOCUS: " << qApp->focusWidget());
	QPushButton * b[4] = { 0, 0, 0, 0 };
	QMessageBox msg_box(QMessageBox::Information,
			toqstr(title), toqstr(question),
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

	if (long_op)
		theApp()->startLongOperation();

	// Qt bug: can return -1 on cancel or WM close, despite the docs.
	if (res == -1)
		res = cancel_button;
	return res;
}

int prompt(docstring const & title0, docstring const & question,
		  int default_button, int cancel_button,
		  docstring const & b1, docstring const & b2,
		  docstring const & b3, docstring const & b4)
{
#ifdef EXPORT_in_THREAD
	return InGuiThread<int>().call(&doPrompt,
#else
	return doPrompt(
#endif
				title0, question, default_button,
				cancel_button, b1, b2, b3, b4);
}

void doWarning(docstring const & title0, docstring const & message,
	     bool const & askshowagain)
{
	lyxerr << "Warning: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(message), QMessageBox::Warning);
		return;
	}

	/// Long operation in progress prevents user from Ok-ing the error dialog
	bool long_op = theApp()->longOperationStarted();
	if (long_op)
		theApp()->stopLongOperation();

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	if (!askshowagain) {
		ProgressInterface::instance()->warning(
				toqstr(title),
				toqstr(message));
	} else {
		ProgressInterface::instance()->toggleWarning(
				toqstr(title),
				toqstr(message),
				toqstr(message));
	}

	qApp->restoreOverrideCursor();

	if (long_op)
		theApp()->startLongOperation();
}

void warning(docstring const & title0, docstring const & message,
	     bool const & askshowagain)
{
#ifdef EXPORT_in_THREAD	
	InGuiThread<void>().call(&doWarning,
#else
	doWarning(
#endif
				title0, message, askshowagain);
}

void doError(docstring const & title0, docstring const & message, bool backtrace)
{
	lyxerr << "Error: " << title0 << '\n'
	       << "----------------------------------------\n"
	       << message << endl;

	QString details;
	if (backtrace) {
		details = QString::fromLocal8Bit(to_local8bit(printCallStack()).c_str());
	}

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(message), QMessageBox::Critical);
		return;
	}

	/// Long operation in progress prevents user from Ok-ing the error dialog
	bool long_op = theApp()->longOperationStarted();
	if (long_op)
		theApp()->stopLongOperation();

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	ProgressInterface::instance()->error(
		toqstr(title),
		toqstr(message),
		details);

	qApp->restoreOverrideCursor();

	if (long_op)
		theApp()->startLongOperation();
}

void error(docstring const & title0, docstring const & message, bool backtrace)
{
#ifdef EXPORT_in_THREAD
	InGuiThread<void>().call(&doError, 
#else
	doError(
#endif
				title0, message, backtrace);
}

void doInformation(docstring const & title0, docstring const & message)
{
	if (!use_gui || lyxerr.debugging())
		lyxerr << title0 << '\n'
		       << "----------------------------------------\n"
		       << message << endl;

	if (!use_gui)
		return;

	docstring const title = bformat(_("LyX: %1$s"), title0);

	if (theApp() == 0) {
		noAppDialog(toqstr(title), toqstr(message), QMessageBox::Information);
		return;
	}

	/// Long operation in progress prevents user from Ok-ing the error dialog
	bool long_op = theApp()->longOperationStarted();
	if (long_op)
		theApp()->stopLongOperation();

	// Don't use a hourglass cursor while displaying the alert
	qApp->setOverrideCursor(Qt::ArrowCursor);

	ProgressInterface::instance()->information(
		toqstr(title),
		toqstr(message));

	qApp->restoreOverrideCursor();

	if (long_op)
		theApp()->startLongOperation();
}

void information(docstring const & title0, docstring const & message)
{
#ifdef EXPORT_in_THREAD
	InGuiThread<void>().call(&doInformation,
#else
	doInformation(
#endif
				title0, message);
}

bool doAskForText(docstring & response, docstring const & msg,
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

	/// Long operation in progress prevents user from Ok-ing the error dialog
	bool long_op = theApp()->longOperationStarted();
	if (long_op)
		theApp()->stopLongOperation();

	bool ok;
	QString text = QInputDialog::getText(qApp->focusWidget(),
		toqstr(title),
		toqstr(char_type('&') + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (long_op)
		theApp()->startLongOperation();

	if (ok) {
		response = qstring_to_ucs4(text);
		return true;
	}
	response.clear();
	return false;
}

bool askForText(docstring & response, docstring const & msg,
	docstring const & dflt)
{
#ifdef EXPORT_in_THREAD
	return InGuiThread<bool>().call(&doAskForText,
#else
	return doAskForText(
#endif
				response, msg, dflt);
}

} // namespace Alert
} // namespace frontend
} // namespace lyx
