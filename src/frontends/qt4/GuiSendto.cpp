/**
 * \file GuiSendto.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSendto.h"

#include "ControlSendto.h"
#include "qt_helpers.h"

#include "Format.h"

#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>

using std::vector;
using std::string;


namespace lyx {
namespace frontend {

GuiSendtoDialog::GuiSendtoDialog(LyXView & lv)
	: GuiDialog(lv, "sendto")
{
	setupUi(this);
	setViewTitle(_("Send Document to Command"));
	setController(new ControlSendto(*this));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(formatLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(slotFormatHighlighted(QListWidgetItem *)));
	connect(formatLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(slotFormatSelected(QListWidgetItem *)));
	connect(formatLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(changed_adaptor()));
	connect(commandCO, SIGNAL(textChanged(const QString&)),
		this, SLOT(changed_adaptor()));

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
}


ControlSendto & GuiSendtoDialog::controller() const
{
	return static_cast<ControlSendto &>(GuiDialog::controller());
}


void GuiSendtoDialog::changed_adaptor()
{
	changed();
}


void GuiSendtoDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiSendtoDialog::update_contents()
{
	all_formats_ = controller().allFormats();

	// Check whether the current contents of the browser will be
	// changed by loading the contents of formats
	vector<string> keys;
	keys.resize(all_formats_.size());

	vector<string>::iterator result = keys.begin();
	vector<Format const *>::const_iterator it  = all_formats_.begin();
	vector<Format const *>::const_iterator end = all_formats_.end();
	for (; it != end; ++it, ++result)
		*result = (*it)->prettyname();

	// Reload the browser
	formatLW->clear();

	for (vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it) {
		formatLW->addItem(toqstr(*it));
	}

	commandCO->addItem(toqstr(controller().getCommand()));
}


void GuiSendtoDialog::applyView()
{
	int const line = formatLW->currentRow();

	if (line < 0 || line > int(formatLW->count()))
		return;

	controller().setFormat(all_formats_[line]);
	controller().setCommand(fromqstr(commandCO->currentText()));
}


bool GuiSendtoDialog::isValid()
{
	int const line = formatLW->currentRow();

	if (line < 0 || line > int(formatLW->count()))
		return false;

	return formatLW->count() != 0 &&
		!commandCO->currentText().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiSendto_moc.cpp"
