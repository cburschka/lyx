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
#include "qt_helpers.h"

#include <algorithm>

#include "Alert.h"
#include "Alert_pimpl.h"

#include "BoostFormat.h"


using std::pair;
using std::make_pair;


void alert_pimpl(string const & s1, string const & s2, string const & s3)
{
	QMessageBox::warning(0, "LyX",
			     toqstr(s1 + '\n' + '\n' + s2 + '\n' + s3));
}


bool askQuestion_pimpl(string const & s1, string const & s2, string const & s3)
{
	return !(QMessageBox::information(0, "LyX", toqstr(s1 + '\n' + s2 + '\n' + s3),
		qt_("&Yes"), qt_("&No"), 0, 1));
}


int askConfirmation_pimpl(string const & s1, string const & s2, string const & s3)
{
	return (QMessageBox::information(0, "LyX", toqstr(s1 + '\n' + s2 + '\n' + s3),
		qt_("&Yes"), qt_("&No"), qt_("&Cancel"), 0, 2)) + 1;
}


pair<bool, string> const
askForText_pimpl(string const & msg, string const & dflt)
{
#if USE_BOOST_FORMAT
	boost::format fmt(qt_("LyX: %1$s"));
	fmt % msg;
	string const title = fmt.str();
#else
	string const title = qt_("LyX: ") + msg;
#endif
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
