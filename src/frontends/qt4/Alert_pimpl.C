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
using lyx::docstring;

using std::pair;
using std::make_pair;


int prompt_pimpl(docstring const & tit, docstring const & question,
		 int default_button, int cancel_button,
		 docstring const & b1, docstring const & b2, docstring const & b3)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

	// FIXME replace that with theApp->gui()->currentView()
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


void warning_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::warning(qApp->focusWidget(),
			     toqstr(title),
			     toqstr(formatted(message)));
}


void error_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::critical(qApp->focusWidget(),
			      toqstr(title),
			      toqstr(formatted(message)));
}


void information_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::information(qApp->focusWidget(),
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
		toqstr(lyx::char_type('&') + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (ok && !text.isEmpty())
		return make_pair<bool, docstring>(true, qstring_to_ucs4(text));
	else
		return make_pair<bool, docstring>(false, docstring());
}
