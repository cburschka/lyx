/***************************************************************************
                          formcopyright.cpp  -  description
                             -------------------
    begin                : Thu Feb 3 2000
    copyright            : (C) 2000 by Jürgen Vigna, 2001 by Kalle Dalheimer
    email                : kalle@klaralvdalens-datakonsult.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include "gettext.h"
#include "FormCopyrightDialogImpl.h"
#include <qlabel.h>
#include <qpushbutton.h>
#undef emit
#include "ButtonControllerBase.h"
#include "qt2BC.h"
#include "ControlCopyright.h"
#include "FormCopyright.h"

typedef Qt2CB<ControlCopyright, Qt2DB<FormCopyrightDialogImpl> > base_class;

FormCopyright::FormCopyright( ControlCopyright& c ) :
    base_class( c, _( "Copyright and Warranty" ) )
{
}


void FormCopyright::build()
{
    // PENDING(kalle) Parent???
    dialog_.reset( new FormCopyrightDialogImpl() );
    connect( dialog_.get()->closePB, SIGNAL( clicked() ),
	     this, SLOT( slotCancel() ) );

    dialog_->copyrightLA->setText( controller().getCopyright().c_str() );
    dialog_->licenseLA->setText( controller().getLicence().c_str() );
    dialog_->disclaimerLA->setText( controller().getDisclaimer().c_str() );

    // Manage the cancel/close button
    bc().setCancel(dialog_->closePB);
    bc().refresh();
}



