/**
 * \file Toolbars.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/Toolbars.h"

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

Toolbars::Toolbars()
{}

#define TurnOnFlag(x)   flags |= ToolbarInfo::x
#define TurnOffFlag(x)  flags &= ~ToolbarInfo::x

void Toolbars::initFlags(ToolbarInfo & tbinfo)
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


void Toolbars::init()
{
	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::iterator end = toolbarbackend.end();

	// init flags will also add these toolbars to session if they
	// are not already there (e.g. first run of lyx).
	for (; cit != end; ++cit)
		initFlags(*cit);

	// add toolbars according the order in session
	ToolbarSection::ToolbarList::const_iterator tb = LyX::ref().session().toolbars().begin();
	ToolbarSection::ToolbarList::const_iterator te = LyX::ref().session().toolbars().end();
	ToolbarSection::ToolbarInfo::Location last_loc = ToolbarSection::ToolbarInfo::NOTSET;
	int last_posx = 0;
	int last_posy = 0;
	for (; tb != te; ++tb) {
		LYXERR(Debug::INIT) << "Adding " << tb->get<0>() << " at position " << tb->get<1>().posx << " " << tb->get<1>().posy << endl;
		// add toolbar break if posx or posy changes
		bool newline = tb->get<1>().location == last_loc && (
			// if two toolbars at the same location, assume uninitialized and add toolbar break
			(tb->get<1>().posx == last_posx && tb->get<1>().posy == last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::TOP && tb->get<1>().posy != last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::BOTTOM && tb->get<1>().posy != last_posy) ||
			(last_loc == ToolbarSection::ToolbarInfo::LEFT && tb->get<1>().posx != last_posx) ||
			(last_loc == ToolbarSection::ToolbarInfo::RIGHT && tb->get<1>().posx != last_posx) );
		// find the backend item and add
		for (cit = toolbarbackend.begin(); cit != end; ++cit)
			if (cit->name == tb->get<0>()) {
				add(*cit, newline);
				last_loc = tb->get<1>().location;
				last_posx = tb->get<1>().posx;
				last_posy = tb->get<1>().posy;
				break;
			}
	}
}


void Toolbars::display(string const & name, bool show)
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


ToolbarInfo * Toolbars::getToolbarInfo(string const & name)
{
	return toolbarbackend.getUsedToolbarInfo(name);
}


void Toolbars::toggleToolbarState(string const & name, bool allowauto)
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


void Toolbars::update(bool in_math, bool in_table, bool review)
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


} // namespace frontend
} // namespace lyx
