/**
 * \file QAbout.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#include <config.h>

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

QAbout::QAbout(ControlAboutlyx & c)
	: base_class(c, _("About LyX"))
{
}


void QAbout::build_dialog()
{
	dialog_.reset(new QAboutDialog());
	connect(dialog_.get()->closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));

	dialog_->copyright->setText(controller().getCopyright().c_str());
	dialog_->copyright->append("\n");
	dialog_->copyright->append(controller().getLicense().c_str());
	dialog_->copyright->append("\n");
	dialog_->copyright->append(controller().getDisclaimer().c_str());

	dialog_->versionLA->setText(controller().getVersion().c_str());

	stringstream in;
	controller().getCredits(in);

	istringstream ss(in.str().c_str());

	string s;
	string out;

	while (getline(ss, s)) {
		if (prefixIs(s, "@b"))
			out += "<b>" + s.substr(2) + "</b>";
		else if (prefixIs(s, "@i"))
			out += "<i>" + s.substr(2) + "</i>";
		else
			out += s;
		out += "<br>";
	}

	dialog_->creditsTV->setText(out.c_str());

	// Manage the cancel/close button
	bc().setCancel(dialog_->closePB);
	bc().refresh();
}
