/**
 * \file QTabular.C
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

#include "ControlTabular.h"
#include "gettext.h"

#include "QTabularDialog.h"
#include "QTabular.h"
#include "Qt2BC.h"

#include <qpushbutton.h>
 
typedef Qt2CB<ControlTabular, Qt2DB<QTabularDialog> > base_class;

QTabular::QTabular()
	: base_class(_("LyX: Edit Table"))
{
}


void QTabular::build_dialog()
{
	dialog_.reset(new QTabularDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QTabular::update_contents()
{
}


bool QTabular::isValid()
{
	return true;
}
