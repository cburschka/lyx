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
#include "debug.h" // temporary


using std::string;
using std::vector;
using std::pair;

namespace lyx {
namespace frontend {

vector<biblio::CiteStyle> ControlCitation::citeStyles_;


ControlCitation::ControlCitation(Dialog & d)
	: ControlCommand(d, "citation")
{}


bool ControlCitation::initialiseParams(string const & data)
{
	lyxerr << "ControlCitation::initialiseParams\n\t" << data << std::endl;
	ControlCommand::initialiseParams(data);

	vector<pair<string, string> > blist;
	kernel().buffer().fillWithBibKeys(blist);

	biblio::CiteEngine const engine = biblio::getEngine(kernel().buffer());

	bool use_styles = engine != biblio::ENGINE_BASIC;

	typedef std::map<string, string>::value_type InfoMapValue;

	for (vector<pair<string,string> >::size_type i = 0;
	     i < blist.size(); ++i) {
		bibkeysInfo_.insert(InfoMapValue(blist[i].first,
						 blist[i].second));
	}

	if (citeStyles_.empty())
		citeStyles_ = biblio::getCiteStyles(engine);
	else {
		if ((use_styles && citeStyles_.size() == 1) ||
		    (!use_styles && citeStyles_.size() != 1))
			citeStyles_ = biblio::getCiteStyles(engine);
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


biblio::CiteEngine_enum ControlCitation::getEngine() const
{
	return biblio::getEngine(kernel().buffer());
}


vector<string> const ControlCitation::getCiteStrings(string const & key) const
{
	vector<string> styles;

	biblio::CiteEngine const engine = biblio::getEngine(kernel().buffer());
	vector<biblio::CiteStyle> const cs = biblio::getCiteStyles(engine);

	if (engine == biblio::ENGINE_NATBIB_NUMERICAL)
		styles = biblio::getNumericalStrings(key, bibkeysInfo_, cs);
	else
		styles = biblio::getAuthorYearStrings(key, bibkeysInfo_, cs);

	return styles;
}

} // namespace frontend
} // namespace lyx
