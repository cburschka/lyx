/**
 * \file QAbout.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#include <config.h>

#include "gettext.h"
#include "QAboutDialog.h"
 
#include <qlabel.h>
#include <qpushbutton.h>
#include "QtLyXView.h"
 
#include "ButtonControllerBase.h"
#include "qt2BC.h"
#include "ControlAboutlyx.h"
#include "QAbout.h"

typedef Qt2CB<ControlAboutlyx, Qt2DB<QAboutDialog> > base_class;

QAbout::QAbout(ControlAboutlyx & c) 
	: base_class(c, _("About LyX"))
{
}


void QAbout::build()
{
	dialog_.reset(new QAboutDialog());
	connect(dialog_.get()->closePB, SIGNAL(clicked()),
		this, SLOT(slotCancel()));

	dialog_->copyrightLA->setText(controller().getCopyright().c_str());
	dialog_->licenseLA->setText(controller().getLicense().c_str());
	dialog_->disclaimerLA->setText(controller().getDisclaimer().c_str());

	// Manage the cancel/close button
	bc().setCancel(dialog_->closePB);
	bc().refresh();
}
