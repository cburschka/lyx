/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 2000 Jean-Marc Lasgouttes
 *
 * ====================================================== */


#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Menubar.h"
#include "MenuBackend.h"
#include "Menubar_pimpl.h"

Menubar::Menubar(LyXView * o, MenuBackend const & md)
{
	pimpl_ = new Pimpl(o, md);
}


Menubar::~Menubar()
{
	delete pimpl_;
}


void Menubar::set(string const & name)
{
	pimpl_->set(name);
}


void Menubar::openByName(string const & name)
{
	pimpl_->openByName(name);
}


void Menubar::update()
{
	pimpl_->update();
}
