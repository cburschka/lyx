/**
 * \file GuiCounter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCounter.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "TextClass.h"
#include "insets/InsetCounter.h"
#include "insets/InsetCommandParams.h"

#include "support/convert.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <map>
//#include <vector>

namespace lyx {
namespace frontend {

GuiCounter::GuiCounter(GuiView & lv, QWidget * parent) : 
	InsetParamsWidget(parent), guiview(lv)
{
	setupUi(this);

	connect(counterCB, SIGNAL(currentIndexChanged(int)),
		this, SIGNAL(changed()));
	connect(actionCB, SIGNAL(currentIndexChanged(int)),
		this, SIGNAL(changed()));
	connect(valueSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(vtypeCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(lyxonlyXB, SIGNAL(clicked()),
		this, SIGNAL(changed()));

	// These are hardcoded and do not change
	std::map<std::string, std::string> const & ct =
			InsetCounter::counterTable;
	actionCB->clear();
	for (auto const & c : ct) {
		docstring guistring = translateIfPossible(from_ascii(c.second));
		actionCB->addItem(toqstr(guistring), toqstr(c.first));
	}

	std::map<std::string, std::string> const & vt =
			InsetCounter::valueTable;
	vtypeCB->clear();
	vtypeCB->addItem("", "");
	for (auto const & v : vt) {
		docstring guistring = translateIfPossible(from_ascii(v.second));
		vtypeCB->addItem(toqstr(guistring), toqstr(v.first));
	}
}


void GuiCounter::processParams(InsetCommandParams const & params)
{
	QString const & counter = toqstr(params["counter"]);
	int c = counterCB->findText(counter);
	counterCB->setCurrentIndex(c);

	QString cmd = toqstr(params.getCmdName());
	c = actionCB->findData(cmd);
	if (c < 0) {
		c = 0;
		LYXERR0("Unable to find " << cmd << " in GuiCounter!");
	}
	actionCB->setCurrentIndex(c);

	int val = convert<int>(params["value"]);
	valueSB->setValue(val);

	cmd = toqstr(params["vtype"]);
	c = cmd.isEmpty() ? 0 : vtypeCB->findData(cmd);
	if (c < 0) {
		c = 0;
		LYXERR0("Unable to find " << cmd << " in GuiCounter!");
	}
	vtypeCB->setCurrentIndex(c);
	lyxonlyXB->setChecked(support::lowercase(params["lyxonly"]) == "true");
}


void GuiCounter::fillCombos()
{
	counterCB->clear();
	BufferView * bv = guiview.documentBufferView();
	// should not happen, but...
	if (!bv)
		return;
	
	std::vector<docstring> counts = 
		bv->buffer().params().documentClass().counters().listOfCounters();
	for (auto const & c : counts)
		counterCB->addItem(toqstr(c));
}


void GuiCounter::paramsToDialog(Inset const * ip)
{
	InsetCounter const * inset = static_cast<InsetCounter const *>(ip);
	InsetCommandParams const & params = inset->params();

	fillCombos();
	processParams(params);
}


bool GuiCounter::initialiseParams(std::string const & data)
{
	InsetCommandParams params(insetCode());
	if (!InsetCommand::string2params(data, params))
		return false;

	fillCombos();
	processParams(params);
	return true;
}


docstring GuiCounter::dialogToParams() const
{
	InsetCommandParams params(insetCode());

	params["counter"] = qstring_to_ucs4(counterCB->currentText());
	params["value"] = convert<docstring>(valueSB->value());
	params.setCmdName(fromqstr(actionCB->currentData().toString()));
	params["vtype"] = qstring_to_ucs4(vtypeCB->currentData().toString());
	params["lyxonly"] = from_ascii(lyxonlyXB->isChecked() ? "true" : "false");
	return from_utf8(InsetCounter::params2string(params));
}


bool GuiCounter::checkWidgets(bool readonly) const
{
	bool const cmdIsValue = actionCB->currentData().toString() == "value";
	bool const cmdIsSet = actionCB->currentData().toString() == "set";
	bool const cmdIsAddTo = actionCB->currentData().toString() == "addto";
	counterCB->setEnabled(!readonly);
	actionCB->setEnabled(!readonly);
	valueSB->setEnabled(!readonly && (cmdIsSet || cmdIsAddTo));
	if (cmdIsAddTo)
		valueSB->setRange(-10000, 10000);
	else
		valueSB->setRange(0, 10000);
	vtypeCB->setEnabled(!readonly && cmdIsValue);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !readonly &&
			!counterCB->currentText().isEmpty() &&
			!actionCB->currentText().isEmpty() &&
			!(cmdIsValue && vtypeCB->currentText().isEmpty());
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiCounter.cpp"
