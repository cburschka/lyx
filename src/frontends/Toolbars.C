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

using std::endl;
using std::string;


Toolbars::Toolbars(LyXView & owner)
	: owner_(owner),
	  layout_(0),
	  last_textclass_(-1)
{}


void Toolbars::init()
{
	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit)
		add(*cit);
}


void Toolbars::display(string const & name, bool show)
{
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->name == name) {
			displayToolbar(*cit, show);
			return;
		}
	}

	lyxerr[Debug::GUI] << "Toolbar::display: no toolbar named "
		<< name << endl;
}


void Toolbars::update(bool in_math, bool in_table)
{
	update();

	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->flags & ToolbarBackend::MATH)
			displayToolbar(*cit, in_math);
		else if (cit->flags & ToolbarBackend::TABLE)
			displayToolbar(*cit, in_table);
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


void Toolbars::add(ToolbarBackend::Toolbar const & tbb)
{
	ToolbarPtr tb_ptr = make_toolbar(tbb, owner_);
	toolbars_[tbb.name] = tb_ptr;

	if (tbb.flags & ToolbarBackend::ON)
		tb_ptr->show(false);
	else
		tb_ptr->hide(false);

	if (tb_ptr->layout())
		layout_ = tb_ptr->layout();
}


void Toolbars::displayToolbar(ToolbarBackend::Toolbar const & tbb,
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

	bool const enable = owner_.getLyXFunc().
		getStatus(FuncRequest(LFUN_LAYOUT)).enabled();

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
		// Yes, the _() is correct
		if (_(itname) == name) {
			FuncRequest const func(LFUN_LAYOUT, itname, 
					       FuncRequest::UI);
			lv.getLyXFunc().dispatch(func);
			return;
		}
	}
	lyxerr << "ERROR (layoutSelected): layout not found!" 
	       << endl;
}


