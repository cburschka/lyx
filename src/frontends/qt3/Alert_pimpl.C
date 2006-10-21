/**
 * \file qt3/Alert_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Alert_pimpl.h"
#include "Alert.h"

#include "ui/QAskForTextDialog.h"
#include "qt_helpers.h"

#include "gettext.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <algorithm>


namespace lyx {

using lyx::docstring;
using lyx::support::bformat;

using std::pair;
using std::make_pair;
using std::string;


int prompt_pimpl(docstring const & tit, docstring const & question,
		 int default_button, int cancel_button,
		 docstring const & b1, docstring const & b2, docstring const & b3)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

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


void warning_pimpl(docstring const & tit, docstring const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::warning(parent,
			     toqstr(title),
			     toqstr(formatted(message)));
}


void error_pimpl(docstring const & tit, docstring const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::critical(parent,
			      toqstr(title),
			      toqstr(formatted(message)));
}


void information_pimpl(docstring const & tit, docstring const & message)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::information(parent,
				 toqstr(title),
				 toqstr(formatted(message)));
}


pair<bool, docstring> const
askForText_pimpl(docstring const & msg, docstring const & dflt)
{
	QWidget * const parent = qApp->focusWidget() ?
		qApp->focusWidget() : qApp->mainWidget();

	docstring const title = bformat(_("LyX: %1$s"), msg);
	QAskForTextDialog d(parent, toqstr(title), true);
        // We try to go with the title set above.
        // d.setCaption(qt_("LyX: Enter text"));

	// less than ideal !
	d.askLA->setText(toqstr('&' + msg));
	d.askLE->setText(toqstr(dflt));
	d.askLE->setFocus();
	int ret = d.exec();

	d.hide();

	if (ret)
		return make_pair<bool, docstring>(true, qstring_to_ucs4(d.askLE->text()));
	else
		return make_pair<bool, docstring>(false, docstring());
}


} // namespace lyx
