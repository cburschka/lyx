// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormCommand.h"

FormCommand::FormCommand( LyXView * lv, Dialogs * d, string const & t )
	: FormBase( lv, d, BUFFER_DEPENDENT, t ),
	  inset_(0), ih_(0)
{}


void FormCommand::showInset( InsetCommand * const inset )
{
	if( dialogIsOpen || inset == 0 ) return;

	inset_ = inset;
	ih_ = inset_->hide.connect(slot(this, &FormCommand::hide));

	params = inset->params();
	show();
}


void FormCommand::createInset( string const & arg )
{
	if( dialogIsOpen ) return;

	params.setFromString( arg );
	show();
}
