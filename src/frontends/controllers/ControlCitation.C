// -*- C++ -*-
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

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlCitation.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"

using std::pair;
using std::vector;
using SigC::slot;

vector<biblio::CiteStyle> ControlCitation::citeStyles_;

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

	if (citeStyles_.empty())
		citeStyles_ = biblio::getCiteStyles(usingNatbib());
	else {
		if ((usingNatbib() && citeStyles_.size() == 1) ||
		    (!usingNatbib() && citeStyles_.size() != 1))
			citeStyles_ = biblio::getCiteStyles(usingNatbib());
	}
}


biblio::InfoMap const & ControlCitation::bibkeysInfo() const
{
	return bibkeysInfo_;
}


bool ControlCitation::usingNatbib() const
{
    return lv_.buffer()->params.use_natbib;
}


vector<string> const ControlCitation::getCiteStrings(string const & key) const
{
	vector<string> styles;

	vector<biblio::CiteStyle> const cs =
		biblio::getCiteStyles(usingNatbib());

	if (lv_.buffer()->params.use_numerical_citations)
		styles = biblio::getNumericalStrings(key, bibkeysInfo_, cs);
	else
		styles = biblio::getAuthorYearStrings(key, bibkeysInfo_, cs);

	return styles;
}
