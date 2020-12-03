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
#include "Converter.h"
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
	: GuiDialog(lv, "sendto", qt_("Export or Send Document")), format_(nullptr)
{
	setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

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
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setApply(buttonBox->button(QDialogButtonBox::Apply));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
}


void GuiSendTo::changed_adaptor()
{
	changed();
}


void GuiSendTo::updateContents()
{
	FormatList const & all_formats =
	    buffer().params().exportableFormats(false);
	// Save the current selection if any
	Format const * current_format = nullptr;
	int const line = formatLW->currentRow();
	if (line >= 0 && static_cast<unsigned int>(line) < all_formats.size()
	    && !formatLW->selectedItems().empty())
		current_format = all_formats[line];
	// Reset the list widget
	formatLW->clear();
	for (Format const * f : all_formats) {
		formatLW->addItem(toqstr(translateIfPossible(f->prettyname())));
		// Restore the selection
		if (current_format && f->prettyname() == current_format->prettyname())
			formatLW->setCurrentRow(formatLW->count() - 1);
	}
}


void GuiSendTo::applyView()
{
	int const line = formatLW->currentRow();
	QString const command = commandCO->currentText().trimmed();

	if (commandCO->findText(command) == -1)
		commandCO->insertItem(0, command);

	if (line < 0 || line > formatLW->count())
		return;

	FormatList const & all_formats =
	    buffer().params().exportableFormats(false);
	format_ = all_formats[line];
	command_ = command;
}


bool GuiSendTo::isValid()
{
	int const line = formatLW->currentRow();

	if (line < 0 || (line > int(formatLW->count())))
		return false;

	return (!formatLW->selectedItems().empty()
		&& formatLW->count() != 0);
}


bool GuiSendTo::initialiseParams(string const &)
{
	format_ = nullptr;
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

	string sdata = format_->name();
	if (!command_.isEmpty())
		sdata += " " + fromqstr(command_);

	FuncCode const lfun = command_.isEmpty() ?
		LFUN_BUFFER_EXPORT : getLfun();

	dispatch(FuncRequest(lfun, sdata));
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiSendto.cpp"
