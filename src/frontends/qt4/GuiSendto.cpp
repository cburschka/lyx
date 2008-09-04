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
#include "Converter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "support/qstring_helpers.h"
#include "support/filetools.h"

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiSendTo::GuiSendTo(GuiView & lv)
	: GuiDialog(lv, "sendto", qt_("Send Document to Command"))
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
	connect(commandCO, SIGNAL(textChanged(QString)),
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
	all_formats_ = allFormats();

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
		&& formatLW->count() != 0
		&& !commandCO->currentText().isEmpty());
}


bool GuiSendTo::initialiseParams(string const &)
{
	format_ = 0;
	command_ = toqstr(lyxrc.custom_export_command);
	paramsToDialog(format_, command_);
	return true;
}


void GuiSendTo::paramsToDialog(Format const * format, QString const & command)
{
	if (!command.isEmpty())
		commandCO->addItem(command);

	bc().setValid(isValid());
}


void GuiSendTo::dispatchParams()
{
	if (command_.isEmpty() || !format_ || format_->name().empty())
		return;

	string const data = format_->name() + " " + fromqstr(command_);
	dispatch(FuncRequest(getLfun(), data));
}

// FIXME: Move to Converters?
vector<Format const *> GuiSendTo::allFormats() const
{
	// What formats can we output natively?
	vector<string> exports;
	exports.push_back("lyx");
	exports.push_back("text");

	if (buffer().isLatex()) {
		exports.push_back("latex");
		exports.push_back("pdflatex");
	}
	else if (buffer().isDocBook())
		exports.push_back("docbook");
	else if (buffer().isLiterate())
		exports.push_back("literate");

	// Loop over these native formats and ascertain what formats we
	// can convert to
	vector<Format const *> to;

	vector<string>::const_iterator ex_it  = exports.begin();
	vector<string>::const_iterator ex_end = exports.end();
	for (; ex_it != ex_end; ++ex_it) {
		// Start off with the native export format.
		// "formats" is LyX's list of recognised formats
		to.push_back(formats.getFormat(*ex_it));

		Formats::const_iterator fo_it  = formats.begin();
		Formats::const_iterator fo_end = formats.end();
		for (; fo_it != fo_end; ++fo_it) {
			// we need to hide the default graphic export formats
			// from the external menu, because we need them only
			// for the internal lyx-view and external latex run
			string const name = fo_it->name();
			if (name != "eps" && name != "xpm" && name != "png" &&
			    theConverters().isReachable(*ex_it, name))
				to.push_back(&(*fo_it));
		}
	}

	// Remove repeated formats.
	sort(to.begin(), to.end());
	to.erase(unique(to.begin(), to.end()), to.end());

	return to;
}


Dialog * createGuiSendTo(GuiView & lv) { return new GuiSendTo(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiSendto_moc.cpp"
