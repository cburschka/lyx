/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#ifdef __GNUG__
#pragma implementation "Toolbar.h"
#endif

#include "Toolbar.h"
#include "Toolbar_pimpl.h"
#include "debug.h"
#include "LyXAction.h"

using std::endl;

extern LyXAction lyxaction;


Toolbar::Toolbar(LyXView * o, int x, int y, ToolbarDefaults const &tbd)
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


void Toolbar::setLayout(int layout)
{
	pimpl_->setLayout(layout);
}


void Toolbar::updateLayoutList(bool force)
{
	pimpl_->updateLayoutList(force);
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
