/**
 * \file ControlSplash.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae
 * \author John Levon
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlSplash.h"
#include "Dialogs.h"
#include "ViewBase.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h" // show_banner

ControlSplash::ControlSplash(Dialogs & d)
	: d_(d)
{
	c_ = d.showSplash.connect(SigC::slot(this, &ControlSplash::show));
}


string const & ControlSplash::bannerFile() const
{
	return banner_file_;
}


string const & ControlSplash::LyXVersion() const
{
	return version_;
}


void ControlSplash::show()
{
	if (!lyxrc.show_banner)
		return;

	banner_file_ = LibFileSearch("images", "banner", "xpm");
	if (banner_file_.empty())
		return;

	version_ = LYX_VERSION;

	view().show();
}


void ControlSplash::hide()
{
	view().hide();
	c_.disconnect();
	d_.destroySplash();
}
