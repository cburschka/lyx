/**
 * \file Menubar_pimpl.C
 * Copyright 1999-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author  Lars Gullik Bjønnes, larsbj@lyx.org
 */

#include <config.h>

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Menubar_pimpl.h"
#include "MenuBackend.h"
#include "LyXAction.h"
#include "kbmap.h"
#include "buffer.h"
#include "lyxfunc.h"
#include "FloatList.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "debug.h"

#include "QtView.h"
#include "QLPopupMenu.h"
 
#include <qmenubar.h>
 
 
using std::endl;
using std::vector;
using std::max;
using std::min;
using std::for_each;

Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mbe) 
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
{
	Menu::const_iterator m = mbe.getMenubar().begin();
	Menu::const_iterator end = mbe.getMenubar().end();
	for (; m != end; ++m) {
		createMenu(owner_->menuBar(), &(*m), this);
	}
}


void Menubar::Pimpl::openByName(string const &)
{
	// FIXME 
}
