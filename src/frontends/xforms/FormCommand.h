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

#ifndef FORMCOMMAND_H
#define FORMCOMMAND_H

#include "FormBase.h"
#include "insets/insetcommand.h"

#ifdef __GNUG__
#pragma interface
#endif

/** This class is an XForms GUI base class to insets derived from
    InsetCommand
 */
class FormCommand : public FormBase {
public:
	/// Constructor
	FormCommand( LyXView *, Dialogs *, string const &,
		     ButtonPolicy * bp = new OkCancelReadOnlyPolicy );

protected:
	/// Slot launching dialog to (possibly) create a new inset
	void createInset( string const & );
	/// Slot launching dialog to an existing inset
	void showInset( InsetCommand * );

	/// pointer to the inset passed through showInset (if any)
	InsetCommand * inset_;
	/// the nitty-griity. What is modified and passed back
  	InsetCommandParams params;
	/// inset::hide connection.
	Connection ih_;
};

#endif
