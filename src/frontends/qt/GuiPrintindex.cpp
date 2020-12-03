/**
 * \file GuiPrintindex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPrintindex.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "IndicesList.h"

#include "support/lstrings.h"

#include "insets/InsetCommand.h"

#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiPrintindex::GuiPrintindex(GuiView & lv)
	: GuiDialog(lv, "index_print", qt_("Index Settings")),
	  params_(insetCode("index_print"))
{
	setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));
	connect(indicesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(subindexCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(literalCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
}


void GuiPrintindex::change_adaptor()
{
	changed();
}


void GuiPrintindex::updateContents()
{
	typedef IndicesList::const_iterator const_iterator;

	IndicesList const & indiceslist = buffer().params().indiceslist();
	docstring const cur_index = suffixIs(params_.getCmdName(), '*') ?
		from_ascii("printall") : params_["type"];

	indicesCO->clear();

	indicesCO->addItem(qt_("<All indexes>"),
			QVariant(QString("printall")));

	const_iterator const begin = indiceslist.begin();
	const_iterator const end = indiceslist.end();
	for (const_iterator it = begin; it != end; ++it)
		indicesCO->addItem(toqstr(it->index()),
			QVariant(toqstr(it->shortcut())));

	int const pos = indicesCO->findData(toqstr(cur_index));
	indicesCO->setCurrentIndex(pos);
	subindexCB->setChecked(params_.getCmdName() == "printsubindex");
	literalCB->setChecked(params_["literal"] == "true");
}


void GuiPrintindex::applyView()
{
	QString const index = indicesCO->itemData(
		indicesCO->currentIndex()).toString();
	string cmd = "printindex";
	if (subindexCB->isChecked())
		cmd = "printsubindex";
	if (index == QString("printall"))
		cmd += '*';
	params_.setCmdName(cmd);
	if (index == QString("printall"))
		params_["type"] = docstring();
	else
		params_["type"] = qstring_to_ucs4(index);
	params_["literal"] = literalCB->isChecked()
			? from_ascii("true") : from_ascii("false");
}


void GuiPrintindex::paramsToDialog(InsetCommandParams const & /*icp*/)
{
	int const pos = suffixIs(params_.getCmdName(), '*') ?
		indicesCO->findData(QString("printall")) :
		indicesCO->findData(toqstr(params_["type"]));
	subindexCB->setChecked(params_.getCmdName() == "printsubindex");
	indicesCO->setCurrentIndex(pos);
	literalCB->setChecked(params_["literal"] == "true");
	bc().setValid(isValid());
}


bool GuiPrintindex::initialiseParams(string const & sdata)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommand::string2params(sdata, params_);
	paramsToDialog(params_);
	return true;
}


void GuiPrintindex::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiPrintindex.cpp"
