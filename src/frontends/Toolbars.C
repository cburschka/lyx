/**
 * \file Toolbars.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Toolbars.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "lyxtextclass.h"
#include "LyXView.h"
#include "lyx_main.h"


namespace lyx {

using std::endl;
using std::string;


Toolbars::Toolbars(LyXView & owner)
	: owner_(owner),
	  layout_(0),
	  last_textclass_(-1)
{}

#define TurnOnFlag(x)   flags |= ToolbarInfo::x
#define TurnOffFlag(x)  flags &= ~ToolbarInfo::x

void Toolbars::initFlags(ToolbarInfo & tbb)
{
	ToolbarSection::ToolbarInfo & info = LyX::ref().session().toolbars().load(tbb.name);

	unsigned int flags = static_cast<unsigned int>(tbb.flags);

	// Remove default.ui positions. Only when a valid postion is stored 
	// in the session file the default.ui value will be overwritten
	unsigned int save = flags;
	TurnOffFlag(TOP);
	TurnOffFlag(BOTTOM);
	TurnOffFlag(RIGHT);
	TurnOffFlag(LEFT);

	bool valid_location = true;
	// init tbb.flags with saved location
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
		// init tbb.flags with saved visibility,
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
	tbb.flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
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
			TurnOffFlag(AUTO);
			if (show) {
				TurnOnFlag(ON);
				TurnOnFlag(OFF);
			} else {
				TurnOnFlag(OFF);
				TurnOnFlag(ON);
			}
			cit->flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
			displayToolbar(*cit, show);
			return;
		}
	}

	LYXERR(Debug::GUI) << "Toolbar::display: no toolbar named "
		<< name << endl;
}


ToolbarInfo::Flags Toolbars::getToolbarState(string const & name)
{	
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->name == name)
			return cit->flags;
	}

	LYXERR(Debug::GUI) << "Toolbar::display: no toolbar named "
		<< name << endl;

	// return dummy for msvc
	return ToolbarInfo::OFF;
}


void Toolbars::toggleToolbarState(string const & name)
{
	ToolbarBackend::Toolbars::iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->name == name) {
			int flags = cit->flags;
			// off -> on
			if (flags & ToolbarInfo::OFF) {
				TurnOffFlag(OFF);
				TurnOnFlag(ON);
			// auto -> off
			} else if (flags & ToolbarInfo::AUTO) {
				TurnOffFlag(AUTO);
				TurnOnFlag(OFF);
			} else if ((flags & ToolbarInfo::MATH) || (flags & ToolbarInfo::TABLE)
				|| (flags & ToolbarInfo::REVIEW)) {
				// for math etc, toggle from on -> auto
				TurnOffFlag(ON);
				TurnOnFlag(AUTO);
			} else {
				// for others, toggle from on -> off
				TurnOffFlag(ON);
				TurnOnFlag(OFF);
			}
			cit->flags = static_cast<lyx::ToolbarInfo::Flags>(flags);
			return;
		}
	}
	LYXERR(Debug::GUI) << "Toolbar::display: no toolbar named "
		<< name << endl;
}
#undef TurnOnFlag
#undef TurnOffFlag


void Toolbars::update(bool in_math, bool in_table, bool review)
{
	update();

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


void Toolbars::saveToolbarInfo()
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


void Toolbars::setLayout(string const & layout)
{
	if (layout_)
		layout_->set(layout);
}


bool Toolbars::updateLayoutList(int textclass)
{
	// update the layout display
	if (last_textclass_ != textclass) {
		if (layout_)
			layout_->update();
		last_textclass_ = textclass;
		return true;
	} else
		return false;
}


void Toolbars::openLayoutList()
{
	if (layout_)
		layout_->open();
}


void Toolbars::clearLayoutList()
{
	last_textclass_ = -1;
	if (layout_)
		layout_->clear();
}


void Toolbars::add(ToolbarInfo const & tbb, bool newline)
{
	ToolbarPtr tb_ptr = owner_.makeToolbar(tbb, newline);
	toolbars_[tbb.name] = tb_ptr;

	if (tbb.flags & ToolbarInfo::ON)
		tb_ptr->show(false);
	else
		tb_ptr->hide(false);

	if (tb_ptr->layout())
		layout_ = tb_ptr->layout();
}


void Toolbars::displayToolbar(ToolbarInfo const & tbb,
			      bool show_it)
{
	ToolbarsMap::iterator it = toolbars_.find(tbb.name);
	BOOST_ASSERT(it != toolbars_.end());

	if (show_it)
		it->second->show(true);
	else
		it->second->hide(true);
}


void Toolbars::update()
{
	ToolbarsMap::const_iterator it = toolbars_.begin();
	ToolbarsMap::const_iterator const end = toolbars_.end();
	for (; it != end; ++it)
		it->second->update();

	bool const enable =
		lyx::getStatus(FuncRequest(LFUN_LAYOUT)).enabled();

	if (layout_)
		layout_->setEnabled(enable);
}


void layoutSelected(LyXView & lv, string const & name)
{
	LyXTextClass const & tc = lv.buffer()->params().getLyXTextClass();

	LyXTextClass::const_iterator it  = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		string const & itname = (*it)->name();
		// Yes, the lyx::to_utf8(_()) is correct
		if (lyx::to_utf8(_(itname)) == name) {
			FuncRequest const func(LFUN_LAYOUT, itname,
					       FuncRequest::TOOLBAR);
			lv.dispatch(func);
			return;
		}
	}
	lyxerr << "ERROR (layoutSelected): layout not found!"
	       << endl;
}


} // namespace lyx
