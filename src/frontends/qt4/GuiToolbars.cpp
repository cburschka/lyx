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

#include "GuiCommandBuffer.h"
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
#include "ToolbarBackend.h"


using std::endl;
using std::string;

namespace lyx {
namespace frontend {

#define TurnOnFlag(x)   flags |= ToolbarInfo::x
#define TurnOffFlag(x)  flags &= ~ToolbarInfo::x

GuiToolbars::GuiToolbars(GuiView & owner)
	: owner_(owner),
	  layout_(0),
	  last_textclass_(TextClassPtr())
{}


void GuiToolbars::initFlags(ToolbarInfo & tbinfo)
{
	ToolbarSection::ToolbarInfo & info = LyX::ref().session().toolbars().load(tbinfo.name);

	unsigned int flags = static_cast<unsigned int>(tbinfo.flags);

	// Remove default.ui positions. Only when a valid postion is stored
	// in the session file the default.ui value will be overwritten
	unsigned int save = flags;
	TurnOffFlag(TOP);
	TurnOffFlag(BOTTOM);
	TurnOffFlag(RIGHT);
	TurnOffFlag(LEFT);

	bool valid_location = true;
	// init tbinfo.flags with saved location
	if (info.location == ToolbarSection::ToolbarInfo::TOP)
		TurnOnFlag(TOP);
	else if (info.location == ToolbarSection::ToolbarInfo::BOTTOM)
		TurnOnFlag(BOTTOM);
	else if (info.location == ToolbarSection::ToolbarInfo::RIGHT)
		TurnOnFlag(RIGHT);
	else if (info.location == ToolbarSection::ToolbarInfo::LEFT)
		TurnOnFlag(LEFT);
	else {
		// use setting from default.ui
		flags = save;
		valid_location = false;
	}

	// invalid location is for a new toolbar that has no saved information,
	// so info.visible is not used for this case.
	if (valid_location) {
		// init tbinfo.flags with saved visibility,
		TurnOffFlag(ON);
		TurnOffFlag(OFF);
		TurnOffFlag(AUTO);
		if (info.state == ToolbarSection::ToolbarInfo::ON)
			TurnOnFlag(ON);
		else if (info.state == ToolbarSection::ToolbarInfo::OFF)
			TurnOnFlag(OFF);
		else
			TurnOnFlag(AUTO);
	}
	/*
	std::cout << "State " << info.state << " FLAGS: " << flags
		<< " ON:" << (flags & ToolbarBackend::ON)
		<< " OFF:" << (flags & ToolbarBackend::OFF)
		<< " L:" << (flags & ToolbarBackend::LEFT)
		<< " R:" << (flags & ToolbarBackend::RIGHT)
		<< " T:" << (flags & ToolbarBackend::TOP)
		<< " B:" << (flags & ToolbarBackend::BOTTOM)
		<< " MA:" << (flags & ToolbarBackend::MATH)
		<< " RE:" << (flags & ToolbarBackend::REVIEW)
		<< " TB:" << (flags & ToolbarBackend::TABLE)
		<< " AU:" << (flags & ToolbarBackend::AUTO)
		<< std::endl;
	*/
	// now set the flags
	tbinfo.flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
}


void GuiToolbars::init()
{
	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::iterator end = toolbarbackend.end();

	// init flags will also add these toolbars to session if they
	// are not already there (e.g. first run of lyx).
	for (; cit != end; ++cit)
		initFlags(*cit);

	// add toolbars according the order in session
	ToolbarSection::ToolbarList::const_iterator tb =
		LyX::ref().session().toolbars().begin();
	ToolbarSection::ToolbarList::const_iterator te =
		LyX::ref().session().toolbars().end();
	ToolbarSection::ToolbarInfo::Location last_loc =
		ToolbarSection::ToolbarInfo::NOTSET;
	int last_posx = 0;
	int last_posy = 0;
	for (; tb != te; ++tb) {
		LYXERR(Debug::INIT) << "Adding " << tb->key << " at position "
			<< tb->info.posx << " " << tb->info.posy << endl;
		// add toolbar break if posx or posy changes
		bool newline = tb->info.location == last_loc && (
			// if two toolbars at the same location, assume uninitialized and add toolbar break
			(tb->info.posx == last_posx && tb->info.posy == last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::TOP && tb->info.posy != last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::BOTTOM && tb->info.posy != last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::LEFT && tb->info.posx != last_posx) ||
			(last_loc == ToolbarSection::ToolbarInfo::RIGHT && tb->info.posx != last_posx) );
		// find the backend item and add
		for (cit = toolbarbackend.begin(); cit != end; ++cit)
			if (cit->name == tb->key) {
				add(*cit, newline);
				last_loc = tb->info.location;
				last_posx = tb->info.posx;
				last_posy = tb->info.posy;
				break;
			}
	}
}


void GuiToolbars::display(string const & name, bool show)
{
	ToolbarBackend::Toolbars::iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->name == name) {
			unsigned int flags = cit->flags;
			TurnOffFlag(ON);
			TurnOffFlag(OFF);
			TurnOffFlag(AUTO);
			if (show)
				TurnOnFlag(ON);
			else
				TurnOnFlag(OFF);
			cit->flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
			displayToolbar(*cit, show);
		}
	}

	LYXERR(Debug::GUI) << "Toolbar::display: no toolbar named "
		<< name << endl;
}


ToolbarInfo * GuiToolbars::getToolbarInfo(string const & name)
{
	return toolbarbackend.getUsedToolbarInfo(name);
}


void GuiToolbars::toggleToolbarState(string const & name, bool allowauto)
{
	ToolbarInfo * tbi = toolbarbackend.getUsedToolbarInfo(name);

	if (!tbi) {
		LYXERR(Debug::GUI) << "Toolbar::display: no toolbar named "
			<< name << endl;
		return;
	}

	int flags = tbi->flags;
	// off -> on
	if (flags & ToolbarInfo::OFF) {
		TurnOffFlag(OFF);
		TurnOnFlag(ON);
	// auto -> off
	} else if (flags & ToolbarInfo::AUTO) {
		TurnOffFlag(AUTO);
		TurnOnFlag(OFF);
	} else if (allowauto 
		   && ((flags & ToolbarInfo::MATH) 
		       || (flags & ToolbarInfo::TABLE)
		       || (flags & ToolbarInfo::REVIEW))) {
		// for math etc, toggle from on -> auto
		TurnOffFlag(ON);
		TurnOnFlag(AUTO);
	} else {
		// for others, toggle from on -> off
		TurnOffFlag(ON);
		TurnOnFlag(OFF);
	}
	tbi->flags = static_cast<ToolbarInfo::Flags>(flags);
}
#undef TurnOnFlag
#undef TurnOffFlag


void GuiToolbars::update(bool in_math, bool in_table, bool review)
{
	updateIcons();

	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->flags & ToolbarInfo::ON)
			displayToolbar(*cit, true);
		else if (cit->flags & ToolbarInfo::OFF)
			displayToolbar(*cit, false);
		else if ((cit->flags & ToolbarInfo::AUTO) && (cit->flags & ToolbarInfo::MATH))
			displayToolbar(*cit, in_math);
		else if ((cit->flags & ToolbarInfo::AUTO) && (cit->flags & ToolbarInfo::TABLE))
			displayToolbar(*cit, in_table);
		else if ((cit->flags & ToolbarInfo::AUTO) && (cit->flags & ToolbarInfo::REVIEW))
			displayToolbar(*cit, review);
	}
}


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


bool GuiToolbars::updateLayoutList(TextClassPtr textclass, bool force)
{
	// update the layout display
	if (last_textclass_ != textclass || force) {
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
		layout_->showPopup();
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


void GuiToolbars::showCommandBuffer(bool show_it)
{
	ToolbarsMap::const_iterator it = toolbars_.begin();
	ToolbarsMap::const_iterator const end = toolbars_.end();
	for (; it != end; ++it) {
		GuiCommandBuffer * cb = it->second->commandBuffer();
		if (!cb)
			continue;
		if (!show_it) {
			// FIXME: this is a hack, "minibuffer" should not be
			// hardcoded.
			display("minibuffer", false);
			return;
		}
		if (!it->second->isVisible())
			display("minibuffer", true);
		cb->setFocus();
		return;
	}
}

} // namespace frontend
} // namespace lyx
