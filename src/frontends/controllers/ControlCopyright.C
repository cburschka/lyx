/*
 * \file ControlCopyright.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlCopyright.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"

using SigC::slot;

ControlCopyright::ControlCopyright(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showCopyright.connect(slot(this, &ControlCopyright::show));
}


string const ControlCopyright::getCopyright() const
{
	return _("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2001 LyX Team");
}

string const ControlCopyright::getLicence() const
{
	return _("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.");
}

string const ControlCopyright::getDisclaimer() const
{
	return _("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");
}
