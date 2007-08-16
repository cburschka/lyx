/**
 * \file ControlCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCitation.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h" // temporary

#include "support/lstrings.h"

#include <boost/regex.hpp>

#include <algorithm>

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
	if (!ControlCommand::initialiseParams(data))
		return false;

	biblio::CiteEngine const engine =
		kernel().buffer().params().getEngine();

	bool use_styles = engine != biblio::ENGINE_BASIC;

	kernel().buffer().fillWithBibKeys(bibkeysInfo_);
	
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


vector<string> const ControlCitation::availableKeys() const
{
	return biblio::getKeys(bibkeysInfo_);
}


biblio::CiteEngine const ControlCitation::getEngine() const
{
	return kernel().buffer().params().getEngine();
}


docstring const ControlCitation::getInfo(std::string const & key) const
{
	if (bibkeysInfo_.empty())
		return docstring();

	return biblio::getInfo(bibkeysInfo_, key);
}

namespace {


// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
// @todo: This function should be moved to support, and then the test in tests
//        should be moved there as well.
docstring const escape_special_chars(docstring const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[' and '\' must be escaped.
	// This is a limitation of boost::regex, but all other chars in BREs
	// are assumed literal.
	boost::regex reg("[].|*?+(){}^$\\[\\\\]");

	// $& is a perl-like expression that expands to all
	// of the current match
	// The '$' must be prefixed with the escape character '\' for
	// boost to treat it as a literal.
	// Thus, to prefix a matched expression with '\', we use:
	// FIXME: UNICODE
	return from_utf8(boost::regex_replace(to_utf8(expr), reg, "\\\\$&"));
}

} // namespace anon

vector<string> ControlCitation::searchKeys(
	vector<string> const & keys_to_search,
	docstring const & search_expression,
	bool case_sensitive, bool regex)
{
	vector<string> foundKeys;

	docstring expr = support::trim(search_expression);
	if (expr.empty())
		return foundKeys;

	if (!regex)
		// We must escape special chars in the search_expr so that
		// it is treated as a simple string by boost::regex.
		expr = escape_special_chars(expr);

	boost::regex reg_exp(to_utf8(expr), case_sensitive ?
		boost::regex_constants::normal : boost::regex_constants::icase);

	vector<string>::const_iterator it = keys_to_search.begin();
	vector<string>::const_iterator end = keys_to_search.end();
	for (; it != end; ++it ) {
		biblio::BibKeyList::const_iterator info = bibkeysInfo_.find(*it);
		if (info == bibkeysInfo_.end())
			continue;
		
		biblio::BibTeXInfo const kvm = info->second;
		string const data = *it + ' ' + to_utf8(kvm.allData);
		
		try {
			if (boost::regex_search(data, reg_exp))
				foundKeys.push_back(*it);
		}
		catch (boost::regex_error &) {
			return vector<string>();
		}
	}
	return foundKeys;
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
