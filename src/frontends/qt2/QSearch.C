/**
 * \file QSearch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "LyXView.h"
#include "ControlSearch.h"
#include "qt_helpers.h"

#include "QSearch.h"
#include "QSearchDialog.h"
#include "Qt2BC.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>


typedef Qt2CB<ControlSearch, Qt2DB<QSearchDialog> > base_class;


QSearch::QSearch()
	: base_class(qt_("LyX: Search Document"))
{
}


void QSearch::build_dialog()
{
	dialog_.reset(new QSearchDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->replaceCO);
	bcview().addReadOnly(dialog_->replacePB);
	bcview().addReadOnly(dialog_->replaceallPB);
	bcview().addReadOnly(dialog_->caseCB);
	bcview().addReadOnly(dialog_->wordsCB);
	bcview().addReadOnly(dialog_->backwardsCB);

	dialog_->replacePB->setEnabled(false);
	dialog_->replaceallPB->setEnabled(false);
}


void QSearch::find(string const & str, bool casesens,
		   bool words, bool backwards)
{
	controller().find(str, casesens, words, !backwards);
}


void QSearch::replace(string const & findstr, string const & replacestr,
	bool casesens, bool words, bool all)
{
	controller().replace(findstr, replacestr, casesens, words, all);
}
