/**
 * \file ControlCredits.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlCredits.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch

// needed for the browser
extern string system_lyxdir;


ControlCredits::ControlCredits(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showCredits.connect(SigC::slot(this, &ControlCredits::show));
}


std::stringstream & ControlCredits::getCredits(std::stringstream & ss) const
{
	string const name = FileSearch(system_lyxdir, "CREDITS");

	bool found(!name.empty());

	if (found) {
		std::ifstream in(name.c_str());
		found = (in.get());

		if (found) {
			in.seekg(0, std::ios::beg); // rewind to the beginning

			ss << in.rdbuf();
			found = (ss.good());
		}
	}

	if (!found) {
		ss << _("ERROR: LyX wasn't able to read CREDITS file\n")
		   << _("Please install correctly to estimate the great\n")
		   << _("amount of work other people have done for the LyX project.");
	}

	return ss;
}
