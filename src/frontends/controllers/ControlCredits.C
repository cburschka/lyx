/**
 * \file ControlCredits.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlCredits.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch

using SigC::slot;
using std::getline;
using std::ifstream;
using std::ios_base;
using std::vector;

ControlCredits::ControlCredits(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showCredits.connect(slot(this, &ControlCredits::show));
}


// needed for the browser
extern string system_lyxdir;

vector<string> const ControlCredits::getCredits() const
{
	vector<string> data;

	string const name = FileSearch(system_lyxdir, "CREDITS");

	bool found = (!name.empty());

	if (found) {
		ifstream in(name.c_str());
		found = (in.get());

		if (found) {
			in.seekg(0, ios_base::beg); // rewind to the beginning

			for(;;) {
				string line;
				getline(in, line);
				if (!in.good()) break;
				data.push_back(line);
			}
		}
	}

	if (!found) {
		data.push_back(_("ERROR: LyX wasn't able to read CREDITS file"));
		data.push_back(_("Please install correctly to estimate the great"));
		data.push_back(_("amount of work other people have done for the LyX project."));
	}

	return data;
}
