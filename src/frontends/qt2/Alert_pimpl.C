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

#ifdef __GNUG__
#pragma implementation
#endif

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "ui/QAskForTextDialog.h"

#include <algorithm>

#include <gettext.h>

#include "Alert.h"
#include "Alert_pimpl.h"

using std::pair;
using std::make_pair;


void alert_pimpl(string const & s1, string const & s2, string const & s3)
{
	QMessageBox::warning(0, "LyX",
			     (s1 + "\n" + "\n" + s2 + "\n" + s3).c_str());
}


bool askQuestion_pimpl(string const & s1, string const & s2, string const & s3)
{
	return !(QMessageBox::information(0, "LyX", (s1 + "\n" + s2 + "\n" + s3).c_str(),
		_("&Yes"), _("&No"), 0, 1));
}


int askConfirmation_pimpl(string const & s1, string const & s2, string const & s3)
{
	return (QMessageBox::information(0, "LyX", (s1 + "\n" + s2 + "\n" + s3).c_str(),
		_("&Yes"), _("&No"), _("&Cancel"), 0, 2)) + 1;
}


pair<bool, string> const
askForText_pimpl(string const & msg, string const & dflt)
{
	string title = _("LyX: ");
	title += msg;

	QAskForTextDialog d(0, title.c_str(), true);
	// less than ideal !
	d.askLA->setText((string("&") + msg).c_str());
	d.askLE->setText(dflt.c_str());
	d.askLE->setFocus();
	int ret = d.exec();

	d.hide();

	if (ret)
		return make_pair<bool, string>(true, d.askLE->text().latin1());
	else
		return make_pair<bool, string>(false, string());
}
