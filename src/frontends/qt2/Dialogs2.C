/**
 * \file qt2/Dialogs2.C
 * Copyright 1995 Matthias Ettrich
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "Dialogs.h"
#include "controllers/GUI.h"
#include "ButtonController.h"

#include "Qt2BC.h"

// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals


struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);
};


Dialogs::Impl::Impl(LyXView &, Dialogs &)
{}


void Dialogs::init_pimpl()
{
	pimpl_ = new Impl(lyxview_, *this);
}


Dialogs::~Dialogs()
{
	delete pimpl_;
}


void Dialogs::showPreamble()
{
	show("document");
	// Oh Angus, won't you help a poor child ?
	//pimpl_->document.view()->showPreamble();
}
