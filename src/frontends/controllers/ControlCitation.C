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
	: ControlCommand(d, "cite", "citation")
{}


bool ControlCitation::initialiseParams(string const & data)
{
	ControlCommand::initialiseParams(data);

	vector<pair<string, docstring> > blist;
	kernel().buffer().fillWithBibKeys(blist);

	biblio::CiteEngine const engine = kernel().buffer().params().getEngine();

	bool use_styles = engine != biblio::ENGINE_BASIC;

	typedef std::map<string, docstring>::value_type InfoMapValue;

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
	return kernel().buffer().params().getEngine();
}


vector<docstring> const ControlCitation::getCiteStrings(string const & key) const
{
	biblio::CiteEngine const engine = kernel().buffer().params().getEngine();
	vector<biblio::CiteStyle> const cs = biblio::getCiteStyles(engine);

	if (engine == biblio::ENGINE_NATBIB_NUMERICAL)
		return biblio::getNumericalStrings(key, bibkeysInfo_, cs);
	else
		return biblio::getAuthorYearStrings(key, bibkeysInfo_, cs);
}

} // namespace frontend
} // namespace lyx
