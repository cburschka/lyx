/**
 * \file qt4/alert_pimpl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Alert_pimpl.h"
#include "alert.h"

#include "ui_AskForTextUi.h"
#include "qt_helpers.h"

#include "frontends/Application.h"

#include "gettext.h"

#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QInputDialog>

#include <algorithm>

using std::pair;
using std::make_pair;
using lyx::support::bformat;

namespace lyx {

namespace {

class MessageBox: public QMessageBox
{
public:
	MessageBox(QWidget * parent = 0) : QMessageBox(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		setAttribute(Qt::WA_QuitOnClose, false);
	}
};

} // anonymous namespace


int prompt_pimpl(docstring const & tit, docstring const & question,
		 int default_button, int cancel_button,
		 docstring const & b1, docstring const & b2, docstring const & b3)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

	MessageBox mb;

	// For some reason, sometimes Qt uses an hourglass or watch cursor when
	// displaying the alert. Hence, we ask for the standard cursor shape.
	// This call has no effect if the cursor has not been overridden.
	qApp->changeOverrideCursor(Qt::ArrowCursor);

	// FIXME replace that with theApp->gui()->currentView()
	int res = mb.information(qApp->focusWidget(),
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


void warning_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

	if (theApp() == 0) {
		int argc = 1;
		char * argv[1];
		QApplication app(argc, argv);
		QMessageBox::warning(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	MessageBox mb;
	mb.warning(qApp->focusWidget(),
			     toqstr(title),
			     toqstr(formatted(message)));
}


void error_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	if (theApp() == 0) {
		int argc = 1;
		char * argv[1];
		QApplication app(argc, argv);
		QMessageBox::critical(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	MessageBox mb;
	mb.critical(qApp->focusWidget(),
			      toqstr(title),
			      toqstr(formatted(message)));
}


void information_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	MessageBox mb;
	mb.information(qApp->focusWidget(),
				 toqstr(title),
				 toqstr(formatted(message)));
}


pair<bool, docstring> const
askForText_pimpl(docstring const & msg, docstring const & dflt)
{
	docstring const title = bformat(_("LyX: %1$s"), msg);

	bool ok;
	QString text = QInputDialog::getText(qApp->focusWidget(),
		toqstr(title),
		toqstr(char_type('&') + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (ok && !text.isEmpty())
		return make_pair<bool, docstring>(true, qstring_to_ucs4(text));
	else
		return make_pair<bool, docstring>(false, docstring());
}


} // namespace lyx
