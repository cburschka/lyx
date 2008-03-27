/**
 * \file InsetCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCitation.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"

#include "frontends/controllers/frontend_helpers.h"

#include "support/fs_extras.h"
#include "support/lstrings.h"

#include <algorithm>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>


namespace lyx {

using support::ascii_lowercase;
using support::contains;
using support::FileName;
using support::getStringFromVector;
using support::getVectorFromString;
using support::ltrim;
using support::rtrim;
using support::split;
using support::tokenPos;

using std::endl;
using std::replace;
using std::string;
using std::ostream;
using std::vector;
using std::map;

namespace fs = boost::filesystem;


namespace {

docstring const getNatbibLabel(Buffer const & buffer,
			    string const & citeType, string const & keyList,
			    docstring const & before, docstring const & after,
			    biblio::CiteEngine engine)
{
	// Only start the process off after the buffer is loaded from file.
	if (!buffer.fully_loaded())
		return docstring();

	// Cache the labels
	typedef std::map<Buffer const *, biblio::InfoMap> CachedMap;
	static CachedMap cached_keys;

	// and cache the timestamp of the bibliography files.
	static std::map<FileName, time_t> bibfileStatus;

	biblio::InfoMap infomap;

	vector<FileName> const & bibfilesCache = buffer.getBibfilesCache();
	// compare the cached timestamps with the actual ones.
	bool changed = false;
	for (vector<FileName>::const_iterator it = bibfilesCache.begin();
			it != bibfilesCache.end(); ++ it) {
		FileName const f = *it;
		try {
			std::time_t lastw = fs::last_write_time(f.toFilesystemEncoding());
			if (lastw != bibfileStatus[f]) {
				changed = true;
				bibfileStatus[f] = lastw;
			}
		}
		catch (fs::filesystem_error & fserr) {
			changed = true;
			lyxerr << "Couldn't find or read bibtex file "
			       << f << endl;
			LYXERR(Debug::DEBUG) << "Fs error: "
					     << fserr.what() << endl;
		}
	}

	// build the keylist only if the bibfiles have been changed
	if (cached_keys[&buffer].empty() || bibfileStatus.empty() || changed) {
		typedef vector<std::pair<string, docstring> > InfoType;
		InfoType bibkeys;
		buffer.fillWithBibKeys(bibkeys);

		InfoType::const_iterator bit  = bibkeys.begin();
		InfoType::const_iterator bend = bibkeys.end();

		for (; bit != bend; ++bit)
			infomap[bit->first] = bit->second;

		cached_keys[&buffer] = infomap;
	} else
		// use the cached keys
		infomap = cached_keys[&buffer];

	if (infomap.empty())
		return docstring();

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

	docstring before_str;
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

	docstring after_str;
	if (!after.empty()) {
		// The "after" key is appended only to the end of the whole.
		after_str = ", " + after;
	}

	// One day, these might be tunable (as they are in BibTeX).
	char const op  = '('; // opening parenthesis.
	char const cp  = ')'; // closing parenthesis.
	// puctuation mark separating citation entries.
	char const * const sep = ";";

	docstring const op_str(' ' + docstring(1, op));
	docstring const cp_str(docstring(1, cp) + ' ');
	docstring const sep_str(from_ascii(sep) + ' ');

	docstring label;
	vector<string> keys = getVectorFromString(keyList);
	vector<string>::const_iterator it  = keys.begin();
	vector<string>::const_iterator end = keys.end();
	for (; it != end; ++it) {
		// get the bibdata corresponding to the key
		docstring const author(biblio::getAbbreviatedAuthor(infomap, *it));
		docstring const year(biblio::getYear(infomap, *it));

		// Something isn't right. Fail safely.
		if (author.empty() || year.empty())
			return docstring();

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
				// FIXME UNICODE
				label += author + op_str + before_str +
					'#' + from_utf8(*it) + cp + sep_str;
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
				// FIXME UNICODE
				label += from_utf8(*it) + sep_str;
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
				// FIXME UNICODE
				label += author + ' ' + before_str +
					'#' + from_utf8(*it) + sep_str;
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
		label = op + label + cp;

	return label;
}


docstring const getBasicLabel(docstring const & keyList, docstring const & after)
{
	docstring keys(keyList);
	docstring label;

	if (contains(keys, ',')) {
		// Final comma allows while loop to cover all keys
		keys = ltrim(split(keys, label, ',')) + ',';
		while (contains(keys, ',')) {
			docstring key;
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


docstring const InsetCitation::generateLabel(Buffer const & buffer) const
{
	docstring const before = getParam("before");
	docstring const after  = getParam("after");

	docstring label;
	biblio::CiteEngine const engine = buffer.params().getEngine();
	if (engine != biblio::ENGINE_BASIC) {
		// FIXME UNICODE
		label = getNatbibLabel(buffer, getCmdName(), to_utf8(getParam("key")),
				       before, after, engine);
	}

	// Fallback to fail-safe
	if (label.empty()) {
		label = getBasicLabel(getParam("key"), after);
	}

	return label;
}


docstring const InsetCitation::getScreenLabel(Buffer const & buffer) const
{
	biblio::CiteEngine const engine = buffer.params().getEngine();
	if (cache.params == params() && cache.engine == engine)
		return cache.screen_label;

	// The label has changed, so we have to re-create it.
	docstring const glabel = generateLabel(buffer);

	unsigned int const maxLabelChars = 45;

	docstring label = glabel;
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


int InsetCitation::plaintext(Buffer const & buffer, odocstream & os,
			     OutputParams const &) const
{
	docstring str;

	if (cache.params == params() &&
	    cache.engine == buffer.params().getEngine())
		str = cache.generated_label;
	else
		str = generateLabel(buffer);

	os << str;
	return str.size();
}


namespace {

docstring const cleanupWhitespace(docstring const & citelist)
{
	docstring::const_iterator it  = citelist.begin();
	docstring::const_iterator end = citelist.end();
	// Paranoia check: make sure that there is no whitespace in here
	// -- at least not behind commas or at the beginning
	docstring result;
	char_type last = ',';
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

int InsetCitation::docbook(Buffer const &, odocstream & os,
			   OutputParams const &) const
{
	os << "<citation>"
	   << cleanupWhitespace(getParam("key"))
	   << "</citation>";
	return 0;
}


void InsetCitation::textString(Buffer const & buf, odocstream & os) const
{
	plaintext(buf, os, OutputParams(0));
}


// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to \cite[]{}
int InsetCitation::latex(Buffer const & buffer, odocstream & os,
			 OutputParams const & op) const
{
	biblio::CiteEngine cite_engine = buffer.params().getEngine();
	// FIXME UNICODE
	docstring const cite_str = from_utf8(
		biblio::asValidLatexCommand(getCmdName(), cite_engine));

	if (op.moving_arg)
		os << "\\protect";

	os << "\\" << cite_str;

	docstring const & before = getParam("before");
	docstring const & after  = getParam("after");
	if (!before.empty() && cite_engine != biblio::ENGINE_BASIC)
		os << '[' << before << "][" << after << ']';
	else if (!after.empty())
		os << '[' << after << ']';

	os << '{' << cleanupWhitespace(getParam("key")) << '}';

	return 0;
}


void InsetCitation::validate(LaTeXFeatures & features) const
{
	switch (features.bufferParams().getEngine()) {
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


void InsetCitation::replaceContents(string const & from, string const & to)
{
	if (tokenPos(getContents(), ',', from) != -1) {
		vector<string> items = getVectorFromString(getContents());
		replace(items.begin(), items.end(), from, to);
		setContents(getStringFromVector(items));
	}
}


} // namespace lyx
