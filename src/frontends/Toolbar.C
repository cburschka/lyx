/**
 * \file Toolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "Toolbar.h"
#include "ToolbarBackend.h"
#include "Toolbar_pimpl.h"
#include "debug.h"
#include "LyXAction.h"

using std::endl;

Toolbar::Toolbar(LyXView * o, int x, int y)
	: last_textclass_(-1)
{
	pimpl_ = new Pimpl(o, x, y);

	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit)
		pimpl_->add(*cit);
}


Toolbar::~Toolbar()
{
	delete pimpl_;
}


void Toolbar::update(bool in_math, bool in_table)
{
	pimpl_->update();

	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		if (cit->flags & ToolbarBackend::MATH)
			pimpl_->displayToolbar(*cit, in_math);
		else if (cit->flags & ToolbarBackend::TABLE)
			pimpl_->displayToolbar(*cit, in_table);
	}
}



void Toolbar::setLayout(string const & layout)
{
	pimpl_->setLayout(layout);
}


bool Toolbar::updateLayoutList(int textclass)
{
	// update the layout display
	if (last_textclass_ != textclass) {
		pimpl_->updateLayoutList(true);
		last_textclass_ = textclass;
		return true;
	} else {
		pimpl_->updateLayoutList(false);
		return false;
	}
}


void Toolbar::openLayoutList()
{
	pimpl_->openLayoutList();
}


void Toolbar::clearLayoutList()
{
	pimpl_->clearLayoutList();
}
