/**
 * \file QTexinfo.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlTexinfo.h"
#include "QTexinfoDialog.h"
#include "QTexinfo.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "helper_funcs.h"

#include "support/lstrings.h"
 
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

using std::vector;

typedef Qt2CB<ControlTexinfo, Qt2DB<QTexinfoDialog> > base_class;

QTexinfo::QTexinfo(ControlTexinfo & c, Dialogs &)
	: base_class(c, _("LaTeX Information")), warningPosted(false), activeStyle(ControlTexinfo::cls)

{
}


void QTexinfo::build_dialog()
{
	dialog_.reset(new QTexinfoDialog(this));

	updateStyles(ControlTexinfo::cls);

	bc().setCancel(dialog_->closePB);
}


void QTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	string const fstr =  controller().getContents(whichStyle, true);
	
	switch (whichStyle) {
	    case ControlTexinfo::bst: 
		bst_ = getVectorFromString(fstr, "\n");
		break;
	    case ControlTexinfo::cls:
		cls_ = getVectorFromString(fstr, "\n");
		break;
	    case ControlTexinfo::sty:
		sty_ = getVectorFromString(fstr, "\n");
		break;
	}

	dialog_->fileList->clear();

	bool const withFullPath = dialog_->path->isChecked();
	string const str =  controller().getContents(whichStyle, withFullPath);
	vector<string> flist = getVectorFromString(str, "\n");
	for (vector<string>::const_iterator fitem = flist.begin();
		fitem != flist.end(); ++fitem) {
		dialog_->fileList->insertItem((*fitem).c_str());
	}

	activeStyle = whichStyle;
}


void QTexinfo::updateStyles()
{
	updateStyles(activeStyle);
}






