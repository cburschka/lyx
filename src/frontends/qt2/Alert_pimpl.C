/**
 * \file qt2/Alert_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "ui/QAskForTextDialog.h"
#include "qt_helpers.h"
#include "gettext.h"

#include <algorithm>

#include "Alert.h"
#include "Alert_pimpl.h"

#include "support/BoostFormat.h"
#include "gettext.h"

using std::pair;
using std::make_pair;
using lyx::support::bformat;

int prompt_pimpl(string const & tit, string const & question,
           int default_button, int cancel_button,
	   string const & b1, string const & b2, string const & b3)
{
	string const title = bformat(_("LyX: %1$s"), tit);

	int res = QMessageBox::information(0, toqstr(title), toqstr(formatted(question)),
		toqstr(b1), toqstr(b2), b3.empty() ? QString::null : toqstr(b3),
		default_button, cancel_button);

	// Qt bug: can return -1 on cancel or WM close, despite the docs.
	if (res == -1)
		res = cancel_button;
	return res;
}


void warning_pimpl(string const & tit, string const & message)
{
	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::warning(0, toqstr(title), toqstr(formatted(message)));
}


void error_pimpl(string const & tit, string const & message)
{
	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::critical(0, toqstr(title), toqstr(formatted(message)));
}


void information_pimpl(string const & tit, string const & message)
{
	string const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::information(0, toqstr(title), toqstr(formatted(message)));
}


pair<bool, string> const
askForText_pimpl(string const & msg, string const & dflt)
{
	string const title = bformat(_("LyX: %1$s"), msg);
	QAskForTextDialog d(0, toqstr(title), true);
	// less than ideal !
	d.askLA->setText(toqstr('&' + msg));
	d.askLE->setText(toqstr(dflt));
	d.askLE->setFocus();
	int ret = d.exec();

	d.hide();

	if (ret)
		return make_pair<bool, string>(true, fromqstr(d.askLE->text()));
	else
		return make_pair<bool, string>(false, string());
}
