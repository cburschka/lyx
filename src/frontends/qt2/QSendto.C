/**
 * \file QSendto.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSendto.h"
#include "QSendtoDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "format.h"

#include "controllers/ControlSendto.h"

#include <qcombobox.h>
#include <qlistbox.h>
#include <qpushbutton.h>

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlSendto, QView<QSendtoDialog> > base_class;


QSendto::QSendto(Dialog & parent)
	: base_class(parent, _("LyX: Send Document to Command"))
{
}


void QSendto::build_dialog()
{
	dialog_.reset(new QSendtoDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
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
		dialog_->formatLB->insertItem(toqstr(*it));
	}

	dialog_->commandCO->insertItem(toqstr(controller().getCommand()));
}


void QSendto::apply()
{
	int const line(dialog_->formatLB->currentItem());

	if (line < 0 || line > int(dialog_->formatLB->count()))
		return;

	string const cmd(fromqstr(dialog_->commandCO->currentText()));

	controller().setFormat(all_formats_[line]);
	controller().setCommand(cmd);
}


bool QSendto::isValid()
{
	int const line(dialog_->formatLB->currentItem());

	if (line < 0 || line > int(dialog_->formatLB->count()))
		return false;

	else return dialog_->formatLB->count() != 0 &&
		!dialog_->commandCO->currentText().isEmpty();
}

} // namespace frontend
} // namespace lyx
