/**
 * \file Toolbar.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes <larsbj@lyx.org>
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

extern LyXAction lyxaction;


Toolbar::Toolbar(LyXView * o, int x, int y, ToolbarDefaults const &tbd)
	: last_textclass_(-1)
{
	pimpl_ = new Pimpl(o, x, y);

	pimpl_->reset();

	// extracts the toolbar actions from tbd
	for (ToolbarDefaults::const_iterator cit = tbd.begin();
	     cit != tbd.end(); ++cit) {
		pimpl_->add((*cit));
		lyxerr[Debug::GUI] << "tool action: "
				       << (*cit) << endl;
	}
}


Toolbar::~Toolbar()
{
	delete pimpl_;
}


void Toolbar::set(bool doingmain)
{
	pimpl_->set(doingmain);
}


void Toolbar::activate()
{
	pimpl_->activate();
}


void Toolbar::deactivate()
{
	pimpl_->deactivate();
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


void Toolbar::push(int nth)
{
	pimpl_->push(nth);
}


void Toolbar::add(string const & func, bool doclean)
{
	int const tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "Toolbar::add: no LyX command called`"
		       << func << "'exists!" << endl;
	} else {
		pimpl_->add(tf, doclean);
	}
}
