/**
 * \file ControlCitation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCitation.h"

#include "buffer.h"
#include "bufferparams.h"


using std::string;
using std::vector;
using std::pair;


vector<biblio::CiteStyle> ControlCitation::citeStyles_;


ControlCitation::ControlCitation(Dialog & d)
	: ControlCommand(d, "citation")
{}


bool ControlCitation::initialiseParams(string const & data)
{
	ControlCommand::initialiseParams(data);

	vector<pair<string, string> > blist;
	kernel().buffer().fillWithBibKeys(blist);

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

	return true;
}



void ControlCitation::clearParams()
{
	ControlCommand::clearParams();
	bibkeysInfo_.clear();
}


biblio::InfoMap const & ControlCitation::bibkeysInfo() const
{
	return bibkeysInfo_;
}


bool ControlCitation::usingNatbib() const
{
    return kernel().buffer().params().use_natbib;
}


vector<string> const ControlCitation::getCiteStrings(string const & key) const
{
	vector<string> styles;

	vector<biblio::CiteStyle> const cs =
		biblio::getCiteStyles(usingNatbib());

	if (kernel().buffer().params().use_numerical_citations)
		styles = biblio::getNumericalStrings(key, bibkeysInfo_, cs);
	else
		styles = biblio::getAuthorYearStrings(key, bibkeysInfo_, cs);

	return styles;
}
