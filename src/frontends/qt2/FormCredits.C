/**
 * \file FormCredits.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 * \author Kalle Dalheimer, kalle@klaralvdalens-datakonsult.se
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gettext.h"
#include "FormCreditsDialogImpl.h"
#include <qtextview.h>
#include <qpushbutton.h>
#undef emit

#include "qt2BC.h"
#include "ControlCredits.h"
#include "FormCredits.h"
#include "Lsstream.h"

using std::getline;

typedef Qt2CB<ControlCredits, Qt2DB<FormCreditsDialogImpl> > base_class;

FormCredits::FormCredits(ControlCredits & c)
	: base_class(c, _("Credits"))
{}


void FormCredits::build()
{
	// PENDING(kalle) Parent?
	dialog_.reset(new FormCreditsDialogImpl( this ));

	// Manage the cancel/close button
	bc().setCancel(dialog_->okPB);
	bc().refresh();

	stringstream ss;
	QString xformscredits = controller().getCredits( ss ).str().c_str();
	QStringList xformslist = QStringList::split( '\n', controller().getCredits( ss ).str().c_str(), true );
	for( QStringList::Iterator it = xformslist.begin(); it != xformslist.end(); ++it ) {
		QString line = *it;
		if( line.left( 2 ) == "@b" )
			dialog_->creditsTV->append( "<b>" + line.mid( 2 ) + "</b>" );
		else if( line.left(  2 ) == "@i" )
			dialog_->creditsTV->append( "<i>" + line.mid( 2 ) + "</i>" );
		else
			dialog_->creditsTV->append( line );
	}
}
