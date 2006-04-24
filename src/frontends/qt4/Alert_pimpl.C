/**
 * \file qt4/Alert_pimpl.C
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
#include "Alert.h"

#include "ui/QAskForTextUi.h"
#include "qt_helpers.h"

#include "gettext.h"

#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QInputDialog>

#include <algorithm>

using lyx::support::bformat;

using std::pair;
using std::make_pair;
using std::string;


int prompt_pimpl(string const & tit, string const & question,
		 int default_button, int cancel_button,
		 string const & b1, string const & b2, string const & b3)
{
	string const title = bformat(_("LyX: %1$s"), tit);

	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	int res = QMessageBox::information(parent,
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


void warning_pimpl(string const & tit, string const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::warning(parent,
			     toqstr(title),
			     toqstr(formatted(message)));
}


void error_pimpl(string const & tit, string const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::critical(parent,
			      toqstr(title),
			      toqstr(formatted(message)));
}


void information_pimpl(string const & tit, string const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::information(parent,
				 toqstr(title),
				 toqstr(formatted(message)));
}


pair<bool, string> const
askForText_pimpl(string const & msg, string const & dflt)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	string const title = bformat(_("LyX: %1$s"), msg);

	bool ok;
	QString text = QInputDialog::getText(parent,
		toqstr(title),
		toqstr('&' + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (ok && !text.isEmpty())
		return make_pair<bool, string>(true, fromqstr(text));
	else
		return make_pair<bool, string>(false, string());
}
