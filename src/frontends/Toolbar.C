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

#ifdef __GNUG__
#pragma implementation "Toolbar.h"
#endif

#include "Toolbar.h"
#include "ToolbarDefaults.h"
#include "Toolbar_pimpl.h"
#include "debug.h"
#include "LyXAction.h"

using std::endl;

Toolbar::Toolbar(LyXView * o, int x, int y, ToolbarDefaults const &tbd)
	: last_textclass_(-1)
{
	pimpl_ = new Pimpl(o, x, y);

	// extracts the toolbar actions from tbd
	for (ToolbarDefaults::const_iterator cit = tbd.begin();
	     cit != tbd.end(); ++cit) {
		pimpl_->add((*cit));
		lyxerr[Debug::GUI] << "tool action: " << (*cit) << endl;
	}
}


Toolbar::~Toolbar()
{
	delete pimpl_;
}


void Toolbar::update()
{
	pimpl_->update();
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
