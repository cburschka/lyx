/**
 * \file GuiToc.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiToc.h"
#include "GuiView.h"
#include "DockView.h"
#include "TocWidget.h"
#include "FuncRequest.h"

#include "insets/InsetCommand.h"

#include "TocModel.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"

#include "support/assert.h"

#include <algorithm>

using namespace std;

namespace lyx {
namespace frontend {

GuiToc::GuiToc(GuiView & parent, Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "toc", qt_("Outline"), area, flags)
{
	widget_ = new TocWidget(*this, &parent);
	setWidget(widget_);
}


GuiToc::~GuiToc()
{
	clearTocModels();
	delete widget_;
}


void GuiToc::clearTocModels()
{
	const unsigned int size = toc_models_.size();
	for (unsigned int i = 0; i < size; ++i) {
		delete toc_models_[i];
	}
	toc_models_.clear();
}


int GuiToc::getTocDepth(int type)
{
	if (type < 0)
		return 0;
	return toc_models_[type]->modelDepth();
}


QStandardItemModel * GuiToc::tocModel(int type)
{
	if (type < 0)
		return 0;

	if (toc_models_.empty()) {
		LYXERR(Debug::GUI, "GuiToc::tocModel(): no types available ");
		return 0;
	}

	LYXERR(Debug::GUI, "GuiToc: type " << type
		<< "  toc_models_.size() " << toc_models_.size());

	LASSERT(type >= 0 && type < int(toc_models_.size()), /**/);
	return toc_models_[type];
}


QModelIndex GuiToc::currentIndex(int type) const
{
	if (type < 0)
		return QModelIndex();

	return toc_models_[type]->modelIndex(currentTocItem(type));
}


void GuiToc::goTo(int type, QModelIndex const & index)
{
	if (type < 0 || !index.isValid()
		|| index.model() != toc_models_[type]) {
		LYXERR(Debug::GUI, "GuiToc::goTo(): QModelIndex is invalid!");
		return;
	}

	LASSERT(type >= 0 && type < int(toc_models_.size()), /**/);

	TocIterator const it = toc_models_[type]->tocIterator(index);

	LYXERR(Debug::GUI, "GuiToc::goTo " << to_utf8(it->str()));

	string const tmp = convert<string>(it->id());
	dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
}


void GuiToc::updateView()
{
	widget_->updateView();
}


TocList const & GuiToc::tocs() const
{
	return buffer().masterBuffer()->tocBackend().tocs();
}


bool GuiToc::initialiseParams(string const & data)
{
	LYXERR(Debug::GUI, data);
	QString str = toqstr(data);
	QString new_type;
	if (str.contains("tableofcontents")) {
		new_type = "tableofcontents";
	} else if (str.contains("floatlist")) {
		if (str.contains("\"figure"))
			new_type = "figure";
		else if (str.contains("\"table"))
			new_type = "table";
		else if (str.contains("\"algorithm"))
			new_type = "algorithm";
	} else if (!str.isEmpty()) {
		new_type = str;
	} else {
		// Default to Outliner.
		new_type = "tableofcontents";
	}

	types_.clear();
	type_names_.clear();
	clearTocModels();
	TocList const & tocs = buffer().masterBuffer()->tocBackend().tocs();
	TocList::const_iterator it = tocs.begin();
	TocList::const_iterator end = tocs.end();
	for (; it != end; ++it) {
		types_.push_back(toqstr(it->first));
		type_names_.push_back(toqstr(guiName(it->first)));
		toc_models_.push_back(new TocModel(it->second));
	}

	widget_->updateGui(types_.indexOf(new_type));

	return true;
}


bool GuiToc::canOutline(int type) const
{
	return types_[type] == "tableofcontents";
}


void GuiToc::outlineUp()
{
	dispatch(FuncRequest(LFUN_OUTLINE_UP));
}


void GuiToc::outlineDown()
{
	dispatch(FuncRequest(LFUN_OUTLINE_DOWN));
}


void GuiToc::outlineIn()
{
	dispatch(FuncRequest(LFUN_OUTLINE_IN));
}


void GuiToc::outlineOut()
{
	dispatch(FuncRequest(LFUN_OUTLINE_OUT));
}


void GuiToc::updateBackend()
{
	buffer().masterBuffer()->tocBackend().update();
	buffer().structureChanged();
}


TocIterator GuiToc::currentTocItem(int type) const
{
	LASSERT(bufferview(), /**/);
	ParConstIterator it(bufferview()->cursor());
	return buffer().masterBuffer()->tocBackend().item(fromqstr(types_[type]), it);
}


docstring GuiToc::guiName(string const & type) const
{
	if (type == "tableofcontents")
		return _("Table of Contents");
	if (type == "child")
		return _("Child Documents");
	if (type == "embedded")
		return _("Embedded Files");
	if (type == "graphics")
		return _("List of Graphics");
	if (type == "equation")
		return _("List of Equations");
	if (type == "footnote")
		return _("List of Foot notes");
	if (type == "listing")
		return _("List of Listings");
	if (type == "index")
		return _("List of Indexes");
	if (type == "marginalnote")
		return _("List of Marginal notes");
	if (type == "note")
		return _("List of Notes");
	if (type == "citation")
		return _("List of Citations");
	if (type == "label")
		return _("Labels and References");

	FloatList const & floats = buffer().params().documentClass().floats();
	if (floats.typeExist(type))
		return _(floats.getType(type).listName());

	return _(type);
}


void GuiToc::dispatchParams()
{
}


Dialog * createGuiToc(GuiView & lv)
{
	GuiView & guiview = static_cast<GuiView &>(lv);
#ifdef Q_WS_MACX
	// On Mac show as a drawer at the right
	return new GuiToc(guiview, Qt::RightDockWidgetArea, Qt::Drawer);
#else
	return new GuiToc(guiview);
#endif
}


} // namespace frontend
} // namespace lyx

#include "GuiToc_moc.cpp"
