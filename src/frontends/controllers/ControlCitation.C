/**
 * \file ControlCitation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlCitation.h"
#include "buffer.h"

using std::vector;
using std::pair;


vector<biblio::CiteStyle> ControlCitation::citeStyles_;


ControlCitation::ControlCitation(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_CITATION_INSERT)
{}


void ControlCitation::clearDaughterParams()
{
	bibkeysInfo_.clear();
}


void ControlCitation::setDaughterParams()
{
	vector<pair<string,string> > blist = buffer()->getBibkeyList();

	typedef std::map<string, string>::value_type InfoMapValue;

	for (vector<pair<string,string> >::size_type i = 0;
	     i < blist.size(); ++i) {
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
    return buffer()->params.use_natbib;
}


vector<string> const ControlCitation::getCiteStrings(string const & key) const
{
	vector<string> styles;

	vector<biblio::CiteStyle> const cs =
		biblio::getCiteStyles(usingNatbib());

	if (buffer()->params.use_numerical_citations)
		styles = biblio::getNumericalStrings(key, bibkeysInfo_, cs);
	else
		styles = biblio::getAuthorYearStrings(key, bibkeysInfo_, cs);

	return styles;
}
