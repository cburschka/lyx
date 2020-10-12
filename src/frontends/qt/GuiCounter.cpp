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
	connect(lyxonlyXB, SIGNAL(clicked()),
		this, SIGNAL(changed()));

	// These are hardcoded and do not change
	std::vector<std::pair<std::string, std::string>> const & ct =
			InsetCounter::counterTable;
	actionCB->clear();
	for (auto const & c : ct) {
		docstring guistring = translateIfPossible(from_ascii(c.second));
		actionCB->addItem(toqstr(guistring), toqstr(c.first));
	}
}


void GuiCounter::processParams(InsetCommandParams const & params)
{
	QString const & counter = toqstr(params["counter"]);
	int c = counterCB->findData(counter);
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

	lyxonlyXB->setChecked(support::lowercase(params["lyxonly"]) == "true");
}


void GuiCounter::fillCombos()
{
	counterCB->clear();
	BufferView * bv = guiview.documentBufferView();
	// should not happen, but...
	if (!bv)
		return;
	
	Counters const & cntrs =
		bv->buffer().params().documentClass().counters();
	std::vector<docstring> counts = cntrs.listOfCounters();
	// We use an intermediate map in order to sort at translated GUI strings.
	QMap<QString, QString> items;
	for (auto const & c : counts) {
		docstring const & guiname = cntrs.guiName(c);
		items.insert(qt_(toqstr(guiname)), toqstr(c));
	}
	for (QMap<QString, QString>::const_iterator it = items.constBegin();
	     it != items.constEnd(); ++it)
		counterCB->addItem(it.key(), it.value());
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

	params["counter"] = qstring_to_ucs4(counterCB->itemData(counterCB->currentIndex()).toString());
	params["value"] = convert<docstring>(valueSB->value());
	params.setCmdName(fromqstr(actionCB->itemData(actionCB->currentIndex()).toString()));
	params["lyxonly"] = from_ascii(lyxonlyXB->isChecked() ? "true" : "false");
	return from_utf8(InsetCounter::params2string(params));
}


bool GuiCounter::checkWidgets(bool readonly) const
{
	bool const cmdIsSet = actionCB->itemData(actionCB->currentIndex()).toString() == "set";
	bool const cmdIsAddTo = actionCB->itemData(actionCB->currentIndex()).toString() == "addto";
	counterCB->setEnabled(!readonly);
	actionCB->setEnabled(!readonly);
	valueSB->setEnabled(!readonly && (cmdIsSet || cmdIsAddTo));
	if (cmdIsAddTo)
		valueSB->setRange(-10000, 10000);
	else
		valueSB->setRange(0, 10000);

	return InsetParamsWidget::checkWidgets() && !readonly &&
			!counterCB->currentText().isEmpty() &&
			!actionCB->currentText().isEmpty();
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiCounter.cpp"
