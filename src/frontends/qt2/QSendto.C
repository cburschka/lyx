/**
 * \file QSendto.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Qt2BC.h"
#include "ControlSendto.h"
#include "QSendtoDialog.h"
#include "QSendto.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include "debug.h"
#include "gettext.h"
#include "converter.h"

using std::vector;

typedef Qt2CB<ControlSendto, Qt2DB<QSendtoDialog> > base_class;


QSendto::QSendto()
	: base_class(_("Send document to command"))
{
}


void QSendto::build_dialog()
{
	dialog_.reset(new QSendtoDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
}


void QSendto::update_contents()
{
	all_formats_ = controller().allFormats();

	// Check whether the current contents of the browser will be
	// changed by loading the contents of formats
	vector<string> keys;
	keys.resize(all_formats_.size());

	vector<string>::iterator result = keys.begin();
	vector<Format const *>::const_iterator it  = all_formats_.begin();
	vector<Format const *>::const_iterator end = all_formats_.end();
	for (; it != end; ++it, ++result) {
		*result = (*it)->prettyname();
	}

	// Reload the browser
	dialog_->formatLB->clear();

	for (vector<string>::const_iterator it = keys.begin();
	     it < keys.end(); ++it) {
		dialog_->formatLB->insertItem(it->c_str());
	}

	dialog_->commandCO->insertItem(controller().getCommand().c_str());
}


void QSendto::apply()
{
	int const line(dialog_->formatLB->currentItem());

	if (line < 0 || line > dialog_->formatLB->count())
		return;

	string const cmd(dialog_->commandCO->currentText().latin1());

	controller().setFormat(all_formats_[line]);
	controller().setCommand(cmd);
}


bool QSendto::isValid()
{
	int const line(dialog_->formatLB->currentItem());

	if (line < 0 || line > dialog_->formatLB->count())
		return false;

	else return dialog_->formatLB->count() != 0 &&
		!string(dialog_->commandCO->currentText()).empty();
}
