/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCitation.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlCitation.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"

using std::pair;
using std::vector;
using SigC::slot;

ControlCitation::ControlCitation(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_CITATION_INSERT)
{
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d_.showCitation.connect(slot(this, &ControlCitation::showInset));
	d_.createCitation.connect(slot(this, &ControlCitation::createInset));
}


void ControlCitation::clearDaughterParams()
{
	bibkeysInfo_.clear();
}


void ControlCitation::setDaughterParams()
{
	vector<pair<string,string> > blist = lv_.buffer()->getBibkeyList();

	typedef std::map<string, string>::value_type InfoMapValue;

	for (vector<pair<string,string> >::size_type i=0; i<blist.size(); ++i) {
		bibkeysInfo_.insert(InfoMapValue(blist[i].first,
						 blist[i].second));
	}
}


biblio::InfoMap const & ControlCitation::bibkeysInfo() const
{
	return bibkeysInfo_;
}
