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

#include "BoostFormat.h"
#include "gettext.h"

using std::pair;
using std::make_pair;


void alert_pimpl(string const & s1, string const & s2, string const & s3)
{
	QMessageBox::warning(0, "LyX",
			     toqstr(s1 + '\n' + '\n' + s2 + '\n' + s3));
}


int prompt_pimpl(string const & tit, string const & question,
           int default_button,
	   string const & b1, string const & b2, string const & b3)
{
#if USE_BOOST_FORMAT
	boost::format fmt(_("LyX: %1$s"));
	fmt % tit;
	string const title = fmt.str();
#else
	string const title = _("LyX: ") + tit;
#endif

	return QMessageBox::information(0, toqstr(title), toqstr(formatted(question)),
		toqstr(b1), toqstr(b2), b3.empty() ? QString::null : toqstr(b3),
		default_button);
}


void warning_pimpl(string const & tit, string const & message)
{
#if USE_BOOST_FORMAT
	boost::format fmt(_("LyX: %1$s"));
	fmt % tit;
	string const title = fmt.str();
#else
	string const title = _("LyX: ") + tit;
#endif
	QMessageBox::warning(0, toqstr(title), toqstr(formatted(message)));
}


void error_pimpl(string const & tit, string const & message)
{
#if USE_BOOST_FORMAT
	boost::format fmt(_("LyX: %1$s"));
	fmt % tit;
	string const title = fmt.str();
#else
	string const title = _("LyX: ") + tit;
#endif
	QMessageBox::critical(0, toqstr(title), toqstr(formatted(message)));
}


void information_pimpl(string const & tit, string const & message)
{
#if USE_BOOST_FORMAT
	boost::format fmt(_("LyX: %1$s"));
	fmt % tit;
	string const title = fmt.str();
#else
	string const title = _("LyX: ") + tit;
#endif
	QMessageBox::information(0, toqstr(title), toqstr(formatted(message)));
}


pair<bool, string> const
askForText_pimpl(string const & msg, string const & dflt)
{
#if USE_BOOST_FORMAT
	boost::format fmt(_("LyX: %1$s"));
	fmt % msg;
	string const title = fmt.str();
#else
	string const title = _("LyX: ") + msg;
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
