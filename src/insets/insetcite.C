/**
 * \file insetcite.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcite.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "LaTeXFeatures.h"

#include "frontends/controllers/biblio.h"

#include "support/lstrings.h"

using lyx::support::ascii_lowercase;
using lyx::support::contains;
using lyx::support::getVectorFromString;
using lyx::support::ltrim;
using lyx::support::rtrim;
using lyx::support::split;

using std::string;
using std::ostream;
using std::vector;
using std::map;

namespace biblio = lyx::biblio;


namespace {

string const getNatbibLabel(Buffer const & buffer,
			    string const & citeType, string const & keyList,
			    string const & before, string const & after,
			    biblio::CiteEngine engine)
{
	// Only start the process off after the buffer is loaded from file.
	if (!buffer.fully_loaded())
		return string();

	typedef std::map<Buffer const *, biblio::InfoMap> CachedMap;
	static CachedMap cached_keys;

	// build the keylist
	typedef vector<std::pair<string, string> > InfoType;
	InfoType bibkeys;
	buffer.fillWithBibKeys(bibkeys);

	InfoType::const_iterator bit  = bibkeys.begin();
	InfoType::const_iterator bend = bibkeys.end();

	biblio::InfoMap infomap;
	for (; bit != bend; ++bit) {
		infomap[bit->first] = bit->second;
	}
	if (infomap.empty())
		return string();

	cached_keys[&buffer] = infomap;

	// the natbib citation-styles
	// CITET:	author (year)
	// CITEP:	(author,year)
	// CITEALT:	author year
	// CITEALP:	author, year
	// CITEAUTHOR:	author
	// CITEYEAR:	year
	// CITEYEARPAR:	(year)
	// jurabib supports these plus
	// CITE:	author/<before field>

	// We don't currently use the full or forceUCase fields.
	string cite_type = biblio::asValidLatexCommand(citeType, engine);
	if (cite_type[0] == 'C')
		cite_type = string(1, 'c') + cite_type.substr(1);
	if (cite_type[cite_type.size() - 1] == '*')
		cite_type = cite_type.substr(0, cite_type.size() - 1);

	string before_str;
	if (!before.empty()) {
		// In CITET and CITEALT mode, the "before" string is
		// attached to the label associated with each and every key.
		// In CITEP, CITEALP and CITEYEARPAR mode, it is attached
		// to the front of the whole only.
		// In other modes, it is not used at all.
		if (cite_type == "citet" ||
		    cite_type == "citealt" ||
		    cite_type == "citep" ||
		    cite_type == "citealp" ||
		    cite_type == "citeyearpar")
			before_str = before + ' ';
		// In CITE (jurabib), the "before" string is used to attach
		// the annotator (of legal texts) to the author(s) of the
		// first reference.
		else if (cite_type == "cite")
			before_str = '/' + before;
	}

	string after_str;
	if (!after.empty()) {
		// The "after" key is appended only to the end of the whole.
		after_str = ", " + after;
	}

	// One day, these might be tunable (as they are in BibTeX).
	char const op  = '('; // opening parenthesis.
	char const cp  = ')'; // closing parenthesis.
	// puctuation mark separating citation entries.
	char const * const sep = ";";

	string const op_str(' ' + string(1, op));
	string const cp_str(string(1, cp) + ' ');
	string const sep_str(string(sep) + ' ');

	string label;
	vector<string> keys = getVectorFromString(keyList);
	vector<string>::const_iterator it  = keys.begin();
	vector<string>::const_iterator end = keys.end();
	for (; it != end; ++it) {
		// get the bibdata corresponding to the key
		string const author(biblio::getAbbreviatedAuthor(infomap, *it));
		string const year(biblio::getYear(infomap, *it));

		// Something isn't right. Fail safely.
		if (author.empty() || year.empty())
			return string();

		// authors1/<before>;  ... ;
		//  authors_last, <after>
		if (cite_type == "cite" && engine == biblio::ENGINE_JURABIB) {
			if (it == keys.begin())
				label += author + before_str + sep_str;
			else
				label += author + sep_str;

		// (authors1 (<before> year);  ... ;
		//  authors_last (<before> year, <after>)
		} else if (cite_type == "citet") {
			switch (engine) {
			case biblio::ENGINE_NATBIB_AUTHORYEAR:
				label += author + op_str + before_str +
					year + cp + sep_str;
				break;
			case biblio::ENGINE_NATBIB_NUMERICAL:
				label += author + op_str + before_str +
					'#' + *it + cp + sep_str;
				break;
			case biblio::ENGINE_JURABIB:
				label += before_str + author + op_str +
					year + cp + sep_str;
				break;
			case biblio::ENGINE_BASIC:
				break;
			}

		// author, year; author, year; ...
		} else if (cite_type == "citep" ||
			   cite_type == "citealp") {
			if (engine == biblio::ENGINE_NATBIB_NUMERICAL) {
				label += *it + sep_str;
			} else {
				label += author + ", " + year + sep_str;
			}

		// (authors1 <before> year;
		//  authors_last <before> year, <after>)
		} else if (cite_type == "citealt") {
			switch (engine) {
			case biblio::ENGINE_NATBIB_AUTHORYEAR:
				label += author + ' ' + before_str +
					year + sep_str;
				break;
			case biblio::ENGINE_NATBIB_NUMERICAL:
				label += author + ' ' + before_str +
					'#' + *it + sep_str;
				break;
			case biblio::ENGINE_JURABIB:
				label += before_str + author + ' ' +
					year + sep_str;
				break;
			case biblio::ENGINE_BASIC:
				break;
			}

		// author; author; ...
		} else if (cite_type == "citeauthor") {
			label += author + sep_str;

		// year; year; ...
		} else if (cite_type == "citeyear" ||
			   cite_type == "citeyearpar") {
			label += year + sep_str;
		}
	}
	label = rtrim(rtrim(label), sep);

	if (!after_str.empty()) {
		if (cite_type == "citet") {
			// insert "after" before last ')'
			label.insert(label.size() - 1, after_str);
		} else {
			bool const add =
				!(engine == biblio::ENGINE_NATBIB_NUMERICAL &&
				  (cite_type == "citeauthor" ||
				   cite_type == "citeyear"));
			if (add)
				label += after_str;
		}
	}

	if (!before_str.empty() && (cite_type == "citep" ||
				    cite_type == "citealp" ||
				    cite_type == "citeyearpar")) {
		label = before_str + label;
	}

	if (cite_type == "citep" || cite_type == "citeyearpar")
		label = string(1, op) + label + string(1, cp);

	return label;
}


string const getBasicLabel(string const & keyList, string const & after)
{
	string keys(keyList);
	string label;

	if (contains(keys, ',')) {
		// Final comma allows while loop to cover all keys
		keys = ltrim(split(keys, label, ',')) + ',';
		while (contains(keys, ',')) {
			string key;
			keys = ltrim(split(keys, key, ','));
			label += ", " + key;
		}
	} else
		label = keys;

	if (!after.empty())
		label += ", " + after;

	return '[' + label + ']';
}

} // anon namespace


InsetCitation::InsetCitation(InsetCommandParams const & p)
	: InsetCommand(p, "citation")
{}


string const InsetCitation::generateLabel(Buffer const & buffer) const
{
	string const before = getSecOptions();
	string const after  = getOptions();

	string label;
	biblio::CiteEngine const engine = buffer.params().cite_engine;
	if (engine != biblio::ENGINE_BASIC) {
		label = getNatbibLabel(buffer, getCmdName(), getContents(),
				       before, after, engine);
	}

	// Fallback to fail-safe
	if (label.empty()) {
		label = getBasicLabel(getContents(), after);
	}

	return label;
}


string const InsetCitation::getScreenLabel(Buffer const & buffer) const
{
	biblio::CiteEngine const engine = biblio::getEngine(buffer);
	if (cache.params == params() && cache.engine == engine)
		return cache.screen_label;

	// The label has changed, so we have to re-create it.
	string const before = getSecOptions();
	string const after  = getOptions();

	string const glabel = generateLabel(buffer);

	unsigned int const maxLabelChars = 45;

	string label = glabel;
	if (label.size() > maxLabelChars) {
		label.erase(maxLabelChars-3);
		label += "...";
	}

	cache.engine  = engine;
	cache.params = params();
	cache.generated_label = glabel;
	cache.screen_label = label;

	return label;
}


int InsetCitation::plaintext(Buffer const & buffer, ostream & os, OutputParams const &) const
{
	if (cache.params == params() &&
	    cache.engine == biblio::getEngine(buffer))
		os << cache.generated_label;
	else
		os << generateLabel(buffer);
	return 0;
}


namespace {
	
string const cleanupWhitespace(string const & citelist)
{
	string::const_iterator it  = citelist.begin();
	string::const_iterator end = citelist.end();
	// Paranoia check: make sure that there is no whitespace in here
	// -- at least not behind commas or at the beginning
	string result;
	char last = ',';
	for (; it != end; ++it) {
		if (*it != ' ')
			last = *it;
		if (*it != ' ' || last != ',')
			result += *it;
	}
	return result;
}

// end anon namyspace
}

int InsetCitation::docbook(Buffer const &, ostream & os, OutputParams const &) const
{
	os << "<citation>" << cleanupWhitespace(getContents()) << "</citation>";
	return 0;
}


// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to \cite[]{}
int InsetCitation::latex(Buffer const & buffer, ostream & os,
			 OutputParams const &) const
{
	biblio::CiteEngine const cite_engine = buffer.params().cite_engine;
	string const cite_str =
		biblio::asValidLatexCommand(getCmdName(), cite_engine);

	os << "\\" << cite_str;

	string const before = getSecOptions();
	string const after  = getOptions();
	if (!before.empty() && cite_engine != biblio::ENGINE_BASIC)
		os << '[' << before << "][" << after << ']';
	else if (!after.empty())
		os << '[' << after << ']';

	os << '{' << cleanupWhitespace(getContents()) << '}';

	return 0;
}


void InsetCitation::validate(LaTeXFeatures & features) const
{
	switch (features.bufferParams().cite_engine) {
	case biblio::ENGINE_BASIC:
		break;
	case biblio::ENGINE_NATBIB_AUTHORYEAR:
	case biblio::ENGINE_NATBIB_NUMERICAL:
		features.require("natbib");
		break;
	case biblio::ENGINE_JURABIB:
		features.require("jurabib");
		break;
	}
}
