/**
 * \file QAbout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
#include "Lsstream.h"
#include "debug.h"
#include "gettext.h"
#include "LyXView.h"
#include "ButtonControllerBase.h"
#include "ControlAboutlyx.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextview.h>

#include "QAboutDialog.h"
#include "Qt2BC.h"
#include "QAbout.h"

using std::getline;

typedef Qt2CB<ControlAboutlyx, Qt2DB<QAboutDialog> > base_class;


QAbout::QAbout()
	: base_class(_("About LyX"))
{
}


void QAbout::build_dialog()
{
	dialog_.reset(new QAboutDialog);
	connect(dialog_.get()->closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));

	dialog_->copyright->setText(controller().getCopyright().c_str());
	dialog_->copyright->append("\n");
	dialog_->copyright->append(controller().getLicense().c_str());
	dialog_->copyright->append("\n");
	dialog_->copyright->append(controller().getDisclaimer().c_str());

	dialog_->versionLA->setText(controller().getVersion().c_str());

	// The code below should depend on a autoconf test. (Lgb)
#if 0
	// There are a lot of buggy stringstream implementations..., but the
	// code below will work on all of them (I hope). The drawback with
	// this solutions os the extra copying. (Lgb)

	ostringstream in;
	controller().getCredits(in);

	istringstream ss(in.str().c_str());

	string s;
	ostringstream out;

	while (getline(ss, s)) {
		if (prefixIs(s, "@b"))
			out << "<b>" << s.substr(2) << "</b>";
		else if (prefixIs(s, "@i"))
			out << "<i>" << s.substr(2) << "</i>";
		else
			out << s;
		out << "<br>";
	}
#else
	// Good stringstream implementations can handle this. It avoids
	// some copying, and should thus be faster and use less memory. (Lgb)
	// I'll make this the default for a short while to see if anyone
	// see the error...
	stringstream in;
	controller().getCredits(in);
	in.seekg(0);
	string s;
	ostringstream out;

	while (getline(in, s)) {
		if (prefixIs(s, "@b"))
			out << "<b>" << s.substr(2) << "</b>";
		else if (prefixIs(s, "@i"))
			out << "<i>" << s.substr(2) << "</i>";
		else
			out << s;
		out << "<br>";
	}
#endif

	dialog_->creditsTV->setText(out.str().c_str());

	// try to resize to a good size
	dialog_->copyright->hide();
	dialog_->setMinimumSize(dialog_->copyright->sizeHint());
	dialog_->copyright->show();
	dialog_->setMinimumSize(dialog_->sizeHint());

	// Manage the cancel/close button
	bc().setCancel(dialog_->closePB);
	bc().refresh();
}
