/**
 * \file Menubar.C
 * Read the file COPYING
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS
 */

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


void Menubar::openByName(string const & name)
{
	pimpl_->openByName(name);
}


void Menubar::update()
{
	pimpl_->update();
}
