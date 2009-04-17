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

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(cancelPB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(indicesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(cancelPB);
}


void GuiIndex::change_adaptor()
{
	changed();
}


void GuiIndex::updateContents()
{
	typedef IndicesList::const_iterator const_iterator;

	IndicesList const & indiceslist = buffer().params().indiceslist();
	docstring const cur_index = params_.index;

	indicesCO->clear();

	const_iterator const begin = indiceslist.begin();
	const_iterator const end = indiceslist.end();
	for (const_iterator it = begin; it != end; ++it)
		indicesCO->addItem(toqstr(it->index()),
			QVariant(toqstr(it->shortcut())));

	int const pos = indicesCO->findData(toqstr(cur_index));
	indicesCO->setCurrentIndex(pos);
}


void GuiIndex::applyView()
{
	QString const index = indicesCO->itemData(
		indicesCO->currentIndex()).toString();
	params_.index = qstring_to_ucs4(index);
}


bool GuiIndex::initialiseParams(string const & data)
{
	InsetIndex::string2params(data, params_);
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


Dialog * createGuiIndex(GuiView & lv) { return new GuiIndex(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiIndex.cpp"
