/**
 * \file QPrefs.C
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

#include "ControlPrefs.h"
#include "QPrefsDialog.h"
#include "QPrefs.h"
#include "Qt2BC.h"
#include "lyxrc.h"

#include <qpushbutton.h>
 
typedef Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> > base_class;


QPrefs::QPrefs()
	: base_class(_("LyX: Preferences"))
{
}


void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));

	// FIXME: wrong
	bc().setOK(dialog_->savePB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);
}


void QPrefs::apply()
{
	LyXRC & rc(controller().rc());

	// do something ... 
}


void QPrefs::update_contents()
{
	LyXRC const & rc(controller().rc());

	// do something ...
}
