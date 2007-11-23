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
#include "debug.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "TextClass.h"

#include "support/convert.h"

#include <algorithm>

using std::endl;
using std::string;


namespace lyx {
namespace frontend {

GuiToc::GuiToc(GuiView & parent, Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "toc", area, flags), params_(TOC_CODE)
{
	widget_ = new TocWidget(*this);
	setWidget(widget_);
	setWindowTitle(widget_->windowTitle());
}


GuiToc::~GuiToc()
{
	delete widget_;
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

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));
	return toc_models_[type];
}


QModelIndex GuiToc::currentIndex(int type) const
{
	if (type < 0)
		return QModelIndex();

	// FIXME: The TocBackend infrastructure is not ready for LOF and LOT
	// This is because a proper ParConstIterator is not constructed in
	// InsetCaption::addToToc()
	if(!canOutline(type))
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

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));

	TocIterator const it = toc_models_[type]->tocIterator(index);

	LYXERR(Debug::GUI, "GuiToc::goTo " << to_utf8(it->str()));

	string const tmp = convert<string>(it->id());
	lyxview().dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
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
	InsetCommandMailer::string2params("toc", data, params_);

	types_.clear();
	type_names_.clear();
	toc_models_.clear();
	TocList const & tocs = buffer().masterBuffer()->tocBackend().tocs();
	TocList::const_iterator it = tocs.begin();
	TocList::const_iterator end = tocs.end();
	for (; it != end; ++it) {
		types_.push_back(it->first);
		type_names_.push_back(guiName(it->first));
		toc_models_.push_back(new TocModel(it->second));
	}

	string selected_type ;
	if (params_["type"].empty()) //Then plain toc...
		selected_type = params_.getCmdName();
	else
		selected_type = to_ascii(params_["type"]);
	selected_type_ = -1;
	for (size_t i = 0;  i != types_.size(); ++i) {
		if (selected_type == types_[i]) {
			selected_type_ = i;
			break;
		}
	}

	modelReset();
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
	BOOST_ASSERT(bufferview());
	ParConstIterator it(bufferview()->cursor());
	return buffer().masterBuffer()->tocBackend().item(types_[type], it);
}


docstring GuiToc::guiName(string const & type) const
{
	if (type == "tableofcontents")
		return _("Table of Contents");

	FloatList const & floats = buffer().params().getTextClass().floats();
	if (floats.typeExist(type))
		return _(floats.getType(type).listName());

	return _(type);
}


void GuiToc::dispatchParams()
{
	string const lfun = 
		InsetCommandMailer::params2string("toc", params_);
	dispatch(FuncRequest(getLfun(), lfun));
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
