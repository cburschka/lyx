/**
 * \file GuiToc.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiToc.h"

#include "TocModel.h"
#include "qt_helpers.h"

#include "debug.h"

#include "ControlToc.h"

#include <algorithm>

using std::endl;

using std::pair;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {


GuiToc::GuiToc(Dialog & dialog, QObject * parent)
	: QObject(parent), ControlToc(dialog)
{
}


bool GuiToc::canOutline(int type) const
{
	if (type < 0)
		return false;

	return ControlToc::canOutline(type);
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
		LYXERR(Debug::GUI) << "GuiToc::tocModel(): no types available " << endl;
		return 0;
	}

	LYXERR(Debug::GUI)
		<< "GuiToc: type " << type
		<< "  toc_models_.size() " << toc_models_.size()
		<< endl;

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));
	return toc_models_[type];
}


QModelIndex const GuiToc::getCurrentIndex(int type) const
{
	if (type < 0)
		return QModelIndex();

	// FIXME: The TocBackend infrastructure is not ready for LOF and LOT
	// This is because a proper ParConstIterator is not constructed in
	// InsetCaption::addToToc()
	if(!canOutline(type))
		return QModelIndex();

	return toc_models_[type]->modelIndex(getCurrentTocItem(type));
}


void GuiToc::goTo(int type, QModelIndex const & index)
{
	if (type < 0 || !index.isValid()
		|| index.model() != toc_models_[type]) {
		LYXERR(Debug::GUI)
			<< "GuiToc::goTo(): QModelIndex is invalid!"
			<< endl;
		return;
	}

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));

	TocIterator const it = toc_models_[type]->tocIterator(index);

	LYXERR(Debug::GUI) << "GuiToc::goTo " << to_utf8(it->str()) << endl;

	ControlToc::goTo(*it);
}


bool GuiToc::initialiseParams(std::string const & data)
{
	if (!ControlToc::initialiseParams(data))
		return false;
	update();
	modelReset();
	return true;
}


void GuiToc::update()
{
	toc_models_.clear();
	TocList::const_iterator it = tocs().begin();
	TocList::const_iterator end = tocs().end();
	for (; it != end; ++it)
		toc_models_.push_back(new TocModel(it->second));
}


} // namespace frontend
} // namespace lyx

#include "GuiToc_moc.cpp"
