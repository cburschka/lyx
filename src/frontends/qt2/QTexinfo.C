/**
 * \file QTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "QTexinfoDialog.h"
#include "QTexinfo.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "support/filetools.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

using lyx::support::OnlyFilename;

using std::string;


typedef QController<ControlTexinfo, QView<QTexinfoDialog> > base_class;

QTexinfo::QTexinfo(Dialog & parent)
	: base_class(parent, _("LyX: LaTeX Information")),
	  warningPosted(false), activeStyle(ControlTexinfo::cls)
{
}


void QTexinfo::build_dialog()
{
	dialog_.reset(new QTexinfoDialog(this));

	updateStyles(ControlTexinfo::cls);

	bcview().setCancel(dialog_->closePB);
}


void QTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	ContentsType & data = texdata_[whichStyle];
	getTexFileList(whichStyle, data);

	bool const withFullPath = dialog_->path->isChecked();

	dialog_->fileList->clear();
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (; it != end; ++it) {
		string const line = withFullPath ? *it : OnlyFilename(*it);
		dialog_->fileList->insertItem(toqstr(line));
	}

	activeStyle = whichStyle;
}


void QTexinfo::updateStyles()
{
	updateStyles(activeStyle);
}
