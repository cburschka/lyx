/**
 * \file QSearch.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <fstream> 

#ifdef __GNUG__
#pragma implementation
#endif

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
 
#include "QSearchDialog.h"
#include "QSearch.h"
#include "Qt2BC.h"
#include "gettext.h"
 
#include "QtLyXView.h"
#include "ControlSearch.h"

typedef Qt2CB<ControlSearch, Qt2DB<QSearchDialog> > base_class;

QSearch::QSearch(ControlSearch & c)
	: base_class(c, _("Search"))
{
}


void QSearch::build_dialog()
{
	dialog_.reset(new QSearchDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->replaceCO); 
	bc().addReadOnly(dialog_->replacePB); 
	bc().addReadOnly(dialog_->replaceallPB); 
	bc().addReadOnly(dialog_->caseCB);
	bc().addReadOnly(dialog_->wordsCB);
	bc().addReadOnly(dialog_->backwardsCB);

	dialog_->replacePB->setEnabled(false); 
	dialog_->replaceallPB->setEnabled(false); 
}

 
void QSearch::find(string const & str, bool casesens, bool words, bool backwards)
{
	controller().find(str, casesens, words, !backwards);
}

 
void QSearch::replace(string const & findstr, string const & replacestr,
	bool casesens, bool words, bool all)
{
	controller().replace(findstr, replacestr, casesens, words, all);
}
