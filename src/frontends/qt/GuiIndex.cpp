/**
 * \file GuiIndex.cpp
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

#include "GuiIndex.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "IndicesList.h"
#include "insets/InsetIndex.h"

#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiIndex::GuiIndex(GuiView & lv)
	: GuiDialog(lv, "index", qt_("Index Entry Settings"))
{
	setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));
	connect(indicesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	rangeCO->addItem(qt_("None"), InsetIndexParams::None);
	rangeCO->addItem(qt_("Start"), InsetIndexParams::Start);
	rangeCO->addItem(qt_("End"), InsetIndexParams::End);
	connect(rangeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	pageFormatCO->addItem(qt_("Default"), toqstr("default"));
	pageFormatCO->addItem(qt_("Bold"), toqstr("textbf"));
	pageFormatCO->addItem(qt_("Italic"), toqstr("textit"));
	pageFormatCO->addItem(qt_("Emphasized"), toqstr("emph"));
	pageFormatCO->addItem(qt_("Custom"), toqstr("custom"));
	connect(pageFormatCO, SIGNAL(activated(int)), this, SLOT(pageFormatChanged(int)));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
}


void GuiIndex::change_adaptor()
{
	changed();
}


void GuiIndex::pageFormatChanged(int i)
{
	QString const pf = pageFormatCO->itemData(i).toString();
	pageFormatLE->setEnabled(pf == "custom");
	change_adaptor();
}


void GuiIndex::updateContents()
{
	typedef IndicesList::const_iterator const_iterator;

	BufferParams const & bp = buffer().masterBuffer()->params();
	indicesGB->setEnabled(bp.use_indices);

	QString const pf = pageFormatCO->itemData(
		pageFormatCO->currentIndex()).toString();
	pageFormatLE->setEnabled(pf == "custom");

	IndicesList const & indiceslist = bp.indiceslist();
	docstring const cur_index = params_.index;

	indicesCO->clear();

	const_iterator const begin = indiceslist.begin();
	const_iterator const end = indiceslist.end();
	for (const_iterator it = begin; it != end; ++it)
		indicesCO->addItem(toqstr(it->index()),
			QVariant(toqstr(it->shortcut())));

	int pos = indicesCO->findData(toqstr(cur_index));
	indicesCO->setCurrentIndex(pos);

	pos = pageFormatCO->findData(toqstr(params_.pagefmt));
	if (pos == -1) {
		pos = pageFormatCO->findData("custom");
		pageFormatLE->setText(toqstr(params_.pagefmt));
	} else
		pageFormatLE->clear();
	pageFormatCO->setCurrentIndex(pos);

	pos = rangeCO->findData(params_.range);
	rangeCO->setCurrentIndex(pos);
}


void GuiIndex::applyView()
{
	QString const index = indicesCO->itemData(
		indicesCO->currentIndex()).toString();
	int const range = rangeCO->itemData(
		rangeCO->currentIndex()).toInt();
	QString const pagefmt = pageFormatCO->itemData(
		pageFormatCO->currentIndex()).toString();
	params_.index = qstring_to_ucs4(index);
	params_.range = InsetIndexParams::PageRange(range);
	params_.pagefmt = (pagefmt == "custom") 
			? fromqstr(pageFormatLE->text())
			: fromqstr(pagefmt);
}


bool GuiIndex::initialiseParams(string const & sdata)
{
	InsetIndex::string2params(sdata, params_);
	return true;
}


void GuiIndex::clearParams()
{
	params_ = InsetIndexParams();
}


void GuiIndex::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetIndex::params2string(params_)));
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiIndex.cpp"
