/**
 * \file GuiSendto.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSendto.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Format.h"
#include "FuncRequest.h"

#include "support/filetools.h"
#include "support/gettext.h"
#include "support/qstring_helpers.h"

#include <algorithm>

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiSendTo::GuiSendTo(GuiView & lv)
	: GuiDialog(lv, "sendto", qt_("Export or Send Document")), format_(0)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(formatLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(slotFormatHighlighted(QListWidgetItem *)));
	connect(formatLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(slotFormatSelected(QListWidgetItem *)));
	connect(formatLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(changed_adaptor()));
	connect(formatLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(changed_adaptor()));
	connect(commandCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(changed_adaptor()));

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
}


void GuiSendTo::changed_adaptor()
{
	changed();
}


void GuiSendTo::updateContents()
{
	all_formats_ = buffer().params().exportableFormats(false);
	sort(all_formats_.begin(), all_formats_.end(), Format::formatSorter);
	
	// Save the current selection if any
	Format const * current_format = 0;
	int const line = formatLW->currentRow();
	if (line >= 0 && line <= formatLW->count()
	    && formatLW->selectedItems().size() > 0)
		current_format = all_formats_[line];

	// Check whether the current contents of the browser will be
	// changed by loading the contents of formats
	vector<string> keys;
	keys.resize(all_formats_.size());

	vector<string>::iterator result = keys.begin();
	vector<Format const *>::const_iterator it  = all_formats_.begin();
	vector<Format const *>::const_iterator end = all_formats_.end();

	int current_line = -1;
	for (int ln = 0; it != end; ++it, ++result, ++ln) {
		*result = (*it)->prettyname();
		if (current_format 
		    && (*it)->prettyname() == current_format->prettyname())
			current_line = ln;
	}

	// Reload the browser
	formatLW->clear();

	for (vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it) {
		formatLW->addItem(qt_(*it));
	}

	// Restore the selection
	if (current_line > -1)
		formatLW->setCurrentItem(formatLW->item(current_line));
}


void GuiSendTo::applyView()
{
	int const line = formatLW->currentRow();
	QString const command = commandCO->currentText().trimmed();

	if (commandCO->findText(command) == -1)
		commandCO->insertItem(0, command);

	if (line < 0 || line > formatLW->count())
		return;

	format_ = all_formats_[line];
	command_ = command;
}


bool GuiSendTo::isValid()
{
	int const line = formatLW->currentRow();

	if (line < 0 || line > int(formatLW->count()))
		return false;

	return (formatLW->selectedItems().size() > 0
		&& formatLW->count() != 0);
}


bool GuiSendTo::initialiseParams(string const &)
{
	format_ = 0;
	paramsToDialog(format_, command_);
	return true;
}


void GuiSendTo::paramsToDialog(Format const * /*format*/, QString const & command)
{
	if (!command.isEmpty())
		commandCO->addItem(command);

	bc().setValid(isValid());
}


void GuiSendTo::dispatchParams()
{
	if (!format_ || format_->name().empty())
		return;

	string data = format_->name();
	if (!command_.isEmpty())
		data += " " + fromqstr(command_);

	FuncCode const lfun = command_.isEmpty() ?
		LFUN_BUFFER_EXPORT : getLfun();

	dispatch(FuncRequest(lfun, data));
}

Dialog * createGuiSendTo(GuiView & lv) { return new GuiSendTo(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiSendto.cpp"
