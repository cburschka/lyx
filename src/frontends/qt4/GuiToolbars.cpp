/**
 * \file GuiToolbars.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiToolbars.h"

#include "GuiToolbar.h"
#include "GuiView.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Layout.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "TextClass.h"


using std::endl;
using std::string;

namespace lyx {
namespace frontend {

GuiToolbars::GuiToolbars(GuiViewBase & owner)
	: owner_(owner),
	  layout_(0),
	  last_textclass_(TextClassPtr())
{}


bool GuiToolbars::visible(string const & name) const
{
	std::map<string, GuiToolbar *>::const_iterator it =
		toolbars_.find(name);
	if (it == toolbars_.end())
		return false;
	return it->second->isVisible();
}


void GuiToolbars::saveToolbarInfo()
{
	ToolbarSection & tb = LyX::ref().session().toolbars();

	for (ToolbarBackend::Toolbars::iterator cit = toolbarbackend.begin();
		cit != toolbarbackend.end(); ++cit) {
		ToolbarsMap::iterator it = toolbars_.find(cit->name);
		BOOST_ASSERT(it != toolbars_.end());
		// get toolbar info from session.
		ToolbarSection::ToolbarInfo & info = tb.load(cit->name);
		if (cit->flags & ToolbarInfo::ON)
			info.state = ToolbarSection::ToolbarInfo::ON;
		else if (cit->flags & ToolbarInfo::OFF)
			info.state = ToolbarSection::ToolbarInfo::OFF;
		else if (cit->flags & ToolbarInfo::AUTO)
			info.state = ToolbarSection::ToolbarInfo::AUTO;
		// save other information
		// if auto, frontend should *not* set on/off
		it->second->saveInfo(info);
		// maybe it is useful to update flags with real status. I do not know
		/*
		if (!(cit->flags & ToolbarInfo::AUTO)) {
			unsigned int flags = static_cast<unsigned int>(cit->flags);
			flags &= ~(info.state == ToolbarSection::ToolbarInfo::ON ? ToolbarInfo::OFF : ToolbarInfo::ON);
			flags |= (info.state == ToolbarSection::ToolbarInfo::ON ? ToolbarInfo::ON : ToolbarInfo::OFF);
			if (info.state == ToolbarSection::ToolbarInfo::ON)
			cit->flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
		}
		*/
	}
}


void GuiToolbars::setLayout(docstring const & layout)
{
	if (layout_)
		layout_->set(layout);
}


bool GuiToolbars::updateLayoutList(TextClassPtr textclass)
{
	// update the layout display
	if (last_textclass_ != textclass) {
		if (layout_)
			layout_->updateContents();
		last_textclass_ = textclass;
		return true;
	} else
		return false;
}


void GuiToolbars::openLayoutList()
{
	if (layout_)
		layout_->open();
}


void GuiToolbars::clearLayoutList()
{
	last_textclass_ = TextClassPtr();
	if (layout_)
		layout_->clear();
}


void GuiToolbars::add(ToolbarInfo const & tbinfo, bool newline)
{
	GuiToolbar * tb_ptr = owner_.makeToolbar(tbinfo, newline);
	toolbars_[tbinfo.name] = tb_ptr;

	if (tbinfo.flags & ToolbarInfo::ON)
		tb_ptr->show();
	else
		tb_ptr->hide();

	if (tb_ptr->layout())
		layout_ = tb_ptr->layout();
}


void GuiToolbars::displayToolbar(ToolbarInfo const & tbinfo,
			      bool show_it)
{
	ToolbarsMap::iterator it = toolbars_.find(tbinfo.name);
	BOOST_ASSERT(it != toolbars_.end());

	if (show_it) {
		if (it->second->isVisible())
			return;
		it->second->show();
	}
	else if (it->second->isVisible())
		it->second->hide();
}


void GuiToolbars::updateIcons()
{
	ToolbarsMap::const_iterator it = toolbars_.begin();
	ToolbarsMap::const_iterator const end = toolbars_.end();
	for (; it != end; ++it)
		it->second->updateContents();

	bool const enable =
		lyx::getStatus(FuncRequest(LFUN_LAYOUT)).enabled();

	if (layout_)
		layout_->setEnabled(enable);
}

} // namespace frontend
} // namespace lyx
