/**
 * \file insetcite.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author Herbert Voss, voss@lyx.org 2002-03-17
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcite.h"
#include "buffer.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "frontends/LyXView.h"
#include "debug.h"
#include "gettext.h"

#include "frontends/controllers/biblio.h"
#include "frontends/Dialogs.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/os.h"
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <map>

using std::ostream;
using std::vector;
using std::map;

namespace {

// An optimisation. We assume that until the first InsetCitation::edit is
// called, we're loding the buffer and that, therefore, we don't need to
// reload the bibkey list
std::map<Buffer const *, bool> loading_buffer;

string const getNatbibLabel(Buffer const * buffer,
			    string const & citeType, string const & keyList,
			    string const & before, string const & after,
			    bool numerical)
{
	// Only reload the bibkeys if we have to...
	map<Buffer const *, bool>::iterator lit = loading_buffer.find(buffer);
	if (lit != loading_buffer.end())
		loading_buffer[buffer] = true;

	typedef std::map<Buffer const *, biblio::InfoMap> CachedMap;
	static CachedMap cached_keys;

	CachedMap::iterator kit = cached_keys.find(buffer);

	if (!loading_buffer[buffer] || kit == cached_keys.end()) {
		// build the keylist
		typedef vector<std::pair<string, string> > InfoType;
		InfoType bibkeys = buffer->getBibkeyList();

		InfoType::const_iterator bit  = bibkeys.begin();
		InfoType::const_iterator bend = bibkeys.end();

		biblio::InfoMap infomap;
		for (; bit != bend; ++bit) {
			infomap[bit->first] = bit->second;
		}
		if (infomap.empty())
		return string();

		cached_keys[buffer] = infomap;
	}

	biblio::InfoMap infomap = cached_keys[buffer];

	// the natbib citation-styles
	// CITET:	author (year)
	// CITEP:	(author,year)
	// CITEALT:	author year
	// CITEALP:	author, year
	// CITEAUTHOR:	author
	// CITEYEAR:	year
	// CITEYEARPAR:	(year)

	// We don't currently use the full or forceUCase fields.
	// bool const forceUCase = citeType[0] == 'C';
	bool const full = citeType[citeType.size() - 1] == '*';

	string const cite_type = full ?
		ascii_lowercase(citeType.substr(0, citeType.size() - 1)) :
		ascii_lowercase(citeType);

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

	string const op_str(string(1, ' ')  + string(1, op));
	string const cp_str(string(1, cp)   + string(1, ' '));
	string const sep_str(string(sep) + " ");

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

		// (authors1 (<before> year);  ... ;
		//  authors_last (<before> year, <after>)
		if (cite_type == "citet") {
			string const tmp = numerical ? '#' + *it : year;
			label += author + op_str + before_str + tmp +
				cp + sep_str;

		// author, year; author, year; ...
		} else if (cite_type == "citep" ||
			   cite_type == "citealp") {
			if (numerical) {
				label += *it + sep_str;
			} else {
				label += author + ", " + year + sep_str;
			}

		// (authors1 <before> year;
		//  authors_last <before> year, <after>)
		} else if (cite_type == "citealt") {
			string const tmp = numerical ? '#' + *it : year;
			label += author + ' ' + before_str + tmp + sep_str;

		// author; author; ...
		} else if (cite_type == "citeauthor") {
			label += author + sep_str;

		// year; year; ...
		} else if (cite_type == "citeyear" ||
			   cite_type == "citeyearpar") {
			label += year + sep_str;
		}
	}
	label = strip(strip(label), sep);

	if (!after_str.empty()) {
		if (cite_type == "citet") {
			// insert "after" before last ')'
			label.insert(label.size() - 1, after_str);
		} else {
			bool const add = !(numerical &&
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

	if (contains(keys, ",")) {
		// Final comma allows while loop to cover all keys
		keys = frontStrip(split(keys, label, ',')) + ",";
		while (contains(keys, ",")) {
			string key;
			keys = frontStrip(split(keys, key, ','));
			label += ", " + key;
		}
	} else
		label = keys;

	if (!after.empty())
		label += ", " + after;

	return "[" + label + "]";
}

} // anon namespace


InsetCitation::InsetCitation(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


string const InsetCitation::generateLabel(Buffer const * buffer) const
{
	string const before = string();
	string const after  = getOptions();

	string label;
	if (buffer->params.use_natbib) {
		string cmd = getCmdName();
		if (cmd == "cite") {
			// We may be "upgrading" from an older LyX version.
			// If, however, we use "cite" because the necessary
			// author/year info is not present in the biblio
			// database, then getNatbibLabel will exit gracefully
			// and we'll call getBasicLabel.
			if (buffer->params.use_numerical_citations)
				cmd = "citep";
			else
				cmd = "citet";
		}
		label = getNatbibLabel(buffer, cmd, getContents(),
				       before, after,
				       buffer->params.use_numerical_citations);
	}

	// Fallback to fail-safe
	if (label.empty()) {
		label = getBasicLabel(getContents(), after);
	}

	return label;
}


InsetCitation::Cache::Style InsetCitation::getStyle(Buffer const * buffer) const
{
	Cache::Style style = Cache::BASIC;

	if (buffer->params.use_natbib) {
		if (buffer->params.use_numerical_citations) {
			style = Cache::NATBIB_NUM;
		} else {
			style = Cache::NATBIB_AY;
		}
	}

	return style;
}


string const InsetCitation::getScreenLabel(Buffer const * buffer) const
{
	Cache::Style const style = getStyle(buffer);
	if (cache.params == params() && cache.style == style)
		return cache.screen_label;

	// The label has changed, so we have to re-create it.
	string const before = string();
	string const after  = getOptions();

	string const glabel = generateLabel(buffer);

	unsigned int const maxLabelChars = 45;

	string label = glabel;
	if (label.size() > maxLabelChars) {
		label.erase(maxLabelChars-3);
		label += "...";
	}

	cache.style  = style;
	cache.params = params();
	cache.generated_label = glabel;
	cache.screen_label = label;

	return label;
}


void InsetCitation::edit(BufferView * bv, int, int, mouse_button::state)
{
	// A call to edit() indicates that we're no longer loading the
	// buffer but doing some real work.
	// Doesn't matter if there is no bv->buffer() entry in the map.
	loading_buffer[bv->buffer()] = false;

	bv->owner()->getDialogs()->showCitation(this);
}


void InsetCitation::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetCitation::ascii(Buffer const * buffer, ostream & os, int) const
{
	string label;

	if (cache.params == params() && cache.style == getStyle(buffer))
		label = cache.generated_label;
	else
		label = generateLabel(buffer);

	os << label;
	return 0;
}


// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to \cite[]{}
int InsetCitation::latex(Buffer const * buffer, ostream & os,
			bool /*fragile*/, bool/*fs*/) const
{
	os << "\\";
	if (buffer->params.use_natbib)
		os << getCmdName();
	else
		os << "cite";

#warning What is this code supposed to do? (Lgb)

#if 1
	// The current strange code

	string const before = string();
	string const after  = getOptions();
	if (!before.empty() && buffer->params.use_natbib)
		os << "[" << before << "][" << after << "]";
	else if (!after.empty())
		os << "[" << after << "]";
#else
	// and the cleaned up equvalent, should it just be changed? (Lgb)
	string const after  = getOptions();
	if (!after.empty())
		os << "[" << after << "]";
#endif
	string::const_iterator it  = getContents().begin();
	string::const_iterator end = getContents().end();
	// Paranoia check: make sure that there is no whitespace in here
	string content;
	for (; it != end; ++it) {
		if (*it != ' ') content += *it;
	}

	os << "{" << content << "}";

	return 0;
}


void InsetCitation::validate(LaTeXFeatures & features) const
{
	if (features.bufferParams().use_natbib)
		features.require("natbib");
}
