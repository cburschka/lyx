/**
 * \file InsetCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCitation.h"

#include "BiblioInfo.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncCode.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "output_xhtml.h"
#include "ParIterator.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileNameList.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

ParamInfo InsetCitation::param_info_;


InsetCitation::InsetCitation(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{
	buffer().removeBiblioTempFiles();
}


InsetCitation::~InsetCitation()
{
	if (isBufferLoaded())
		buffer().removeBiblioTempFiles();
}


ParamInfo const & InsetCitation::findInfo(string const & /* cmdName */)
{
	// standard cite does only take one argument if jurabib is
	// not used, but jurabib extends this to two arguments, so
	// we have to allow both here. InsetCitation takes care that
	// LaTeX output is nevertheless correct.
	if (param_info_.empty()) {
		param_info_.add("after", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("before", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("key", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


namespace {

vector<string> const init_possible_cite_commands()
{
	char const * const possible[] = {
		"cite", "nocite", "citet", "citep", "citealt", "citealp",
		"citeauthor", "citeyear", "citeyearpar",
		"citet*", "citep*", "citealt*", "citealp*", "citeauthor*",
		"Citet",  "Citep",  "Citealt",  "Citealp",  "Citeauthor",
		"Citet*", "Citep*", "Citealt*", "Citealp*", "Citeauthor*",
		"fullcite",
		"footcite", "footcitet", "footcitep", "footcitealt",
		"footcitealp", "footciteauthor", "footciteyear", "footciteyearpar",
		"citefield", "citetitle", "cite*"
	};
	size_t const size_possible = sizeof(possible) / sizeof(possible[0]);

	return vector<string>(possible, possible + size_possible);
}


vector<string> const & possibleCiteCommands()
{
	static vector<string> const possible = init_possible_cite_commands();
	return possible;
}


} // anon namespace


bool InsetCitation::isCompatibleCommand(string const & cmd)
{
	vector<string> const & possibles = possibleCiteCommands();
	vector<string>::const_iterator const end = possibles.end();
	return find(possibles.begin(), end, cmd) != end;
}


void InsetCitation::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	if (cmd.action() == LFUN_INSET_MODIFY) {
		buffer().removeBiblioTempFiles();
		cache.recalculate = true;
	}
	InsetCommand::doDispatch(cur, cmd);
}


docstring InsetCitation::toolTip(BufferView const & bv, int, int) const
{
	Buffer const & buf = bv.buffer();
	// Only after the buffer is loaded from file...
	if (!buf.isFullyLoaded())
		return docstring();

	BiblioInfo const & bi = buf.masterBibInfo();
	if (bi.empty())
		return _("No bibliography defined!");

	docstring const & key = getParam("key");
	if (key.empty())
		return _("No citations selected!");

	vector<docstring> keys = getVectorFromString(key);
	vector<docstring>::const_iterator it = keys.begin();
	vector<docstring>::const_iterator en = keys.end();
	docstring tip;
	for (; it != en; ++it) {
		docstring const key_info = bi.getInfo(*it, buffer());
		if (key_info.empty())
			continue;
		if (!tip.empty())
			tip += "\n";
		tip += wrap(key_info, -4);
	}
	return tip;
}


namespace {
	
// FIXME See the header for the issue.
string defaultCiteCommand(CiteEngine engine)
{
	string str;
	switch (engine) {
		case ENGINE_BASIC:
			str = "cite";
			break;
		case ENGINE_NATBIB_AUTHORYEAR:
			str = "citet";
			break;
		case ENGINE_NATBIB_NUMERICAL:
			str = "citep";
			break;
		case ENGINE_JURABIB:
			str = "cite";
			break;
	}
	return str;
}

	
string asValidLatexCommand(string const & input, CiteEngine const engine)
{
	string const default_str = defaultCiteCommand(engine);
	if (!InsetCitation::isCompatibleCommand(input))
		return default_str;

	string output;
	switch (engine) {
		case ENGINE_BASIC:
			if (input == "nocite")
				output = input;
			else
				output = default_str;
			break;

		case ENGINE_NATBIB_AUTHORYEAR:
		case ENGINE_NATBIB_NUMERICAL:
			if (input == "cite" || input == "citefield"
			    || input == "citetitle" || input == "cite*")
				output = default_str;
			else if (prefixIs(input, "foot"))
				output = input.substr(4);
			else
				output = input;
			break;

		case ENGINE_JURABIB: {
			// Jurabib does not support the 'uppercase' natbib style.
			if (input[0] == 'C')
				output = string(1, 'c') + input.substr(1);
			else
				output = input;

			// Jurabib does not support the 'full' natbib style.
			string::size_type const n = output.size() - 1;
			if (output != "cite*" && output[n] == '*')
				output = output.substr(0, n);

			break;
		}
	}

	return output;
}


inline docstring wrapCitation(docstring const & key, 
		docstring const & content, bool for_xhtml)
{
	if (!for_xhtml)
		return content;
	// we have to do the escaping here, because we will ultimately
	// write this as a raw string, so as not to escape the tags.
	return "<a href='#" + key + "'>" +
			html::htmlize(content, XHTMLStream::ESCAPE_ALL) + "</a>";
}

} // anonymous namespace

docstring InsetCitation::generateLabel(bool for_xhtml) const
{
	docstring label;
	label = complexLabel(for_xhtml);

	// Fallback to fail-safe
	if (label.empty())
		label = basicLabel(for_xhtml);

	return label;
}


docstring InsetCitation::complexLabel(bool for_xhtml) const
{
	Buffer const & buf = buffer();
	// Only start the process off after the buffer is loaded from file.
	if (!buf.isFullyLoaded())
		return docstring();

	BiblioInfo const & biblist = buf.masterBibInfo();
	if (biblist.empty())
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

	CiteEngine const engine = buffer().params().citeEngine();
	Language const * lang = buffer().params().language;
	// We don't currently use the full or forceUCase fields.
	string cite_type = asValidLatexCommand(getCmdName(), engine);
	if (cite_type[0] == 'C')
		// If we were going to use them, this would mean ForceUCase
		cite_type = string(1, 'c') + cite_type.substr(1);
	if (cite_type[cite_type.size() - 1] == '*')
		// and this would mean FULL
		cite_type = cite_type.substr(0, cite_type.size() - 1);

	docstring const & before = getParam("before");
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

	docstring const & after = getParam("after");
	docstring after_str;
	// The "after" key is appended only to the end of the whole.
	if (cite_type == "nocite")
		after_str =  " (" + _("not cited") + ')';
	else if (!after.empty()) {
		after_str = ", " + after;
	}

	// One day, these might be tunable (as they are in BibTeX).
	char op, cp;	// opening and closing parenthesis.
	const char * sep;	// punctuation mark separating citation entries.
	if (engine == ENGINE_BASIC) {
		op  = '[';
		cp  = ']';
		sep = ",";
	} else {
		op  = '(';
		cp  = ')';
		sep = ";";
	}

	docstring const op_str = ' ' + docstring(1, op);
	docstring const cp_str = docstring(1, cp) + ' ';
	docstring const sep_str = from_ascii(sep) + ' ';

	docstring label;
	vector<docstring> keys = getVectorFromString(getParam("key"));
	vector<docstring>::const_iterator it  = keys.begin();
	vector<docstring>::const_iterator end = keys.end();
	for (; it != end; ++it) {
		// get the bibdata corresponding to the key
		docstring const author = biblist.getAbbreviatedAuthor(*it, lang->code());
		docstring const year = biblist.getYear(*it, for_xhtml, lang->code());
		docstring const citenum = for_xhtml ? biblist.getCiteNumber(*it) : *it;

		if (author.empty() || year.empty())
			// We can't construct a "complex" label without that info.
			// So fail safely.
			return docstring();

		// authors1/<before>;  ... ;
		//  authors_last, <after>
		if (cite_type == "cite") {
			if (engine == ENGINE_BASIC) {
				label += wrapCitation(*it, citenum, for_xhtml) + sep_str;
			} else if (engine == ENGINE_JURABIB) {
				if (it == keys.begin())
					label += wrapCitation(*it, author, for_xhtml) + before_str + sep_str;
				else
					label += wrapCitation(*it, author, for_xhtml) + sep_str;
			}
		} 
		// nocite
		else if (cite_type == "nocite") {
			label += *it + sep_str;
		} 
		// (authors1 (<before> year);  ... ;
		//  authors_last (<before> year, <after>)
		else if (cite_type == "citet") {
			switch (engine) {
			case ENGINE_NATBIB_AUTHORYEAR:
				label += author + op_str + before_str +
					wrapCitation(*it, year, for_xhtml) + cp + sep_str;
				break;
			case ENGINE_NATBIB_NUMERICAL:
				label += author + op_str + before_str + 
					wrapCitation(*it, citenum, for_xhtml) + cp + sep_str;
				break;
			case ENGINE_JURABIB:
				label += before_str + author + op_str +
					wrapCitation(*it, year, for_xhtml) + cp + sep_str;
				break;
			case ENGINE_BASIC:
				break;
			}
		} 
		// author, year; author, year; ...	
		else if (cite_type == "citep" ||
			   cite_type == "citealp") {
			if (engine == ENGINE_NATBIB_NUMERICAL) {
				label += wrapCitation(*it, citenum, for_xhtml) + sep_str;
			} else {
				label += wrapCitation(*it, author + ", " + year, for_xhtml) + sep_str;
			}

		} 
		// (authors1 <before> year;
		//  authors_last <before> year, <after>)
		else if (cite_type == "citealt") {
			switch (engine) {
			case ENGINE_NATBIB_AUTHORYEAR:
				label += author + ' ' + before_str +
					wrapCitation(*it, year, for_xhtml) + sep_str;
				break;
			case ENGINE_NATBIB_NUMERICAL:
				label += author + ' ' + before_str + '#' + 
					wrapCitation(*it, citenum, for_xhtml) + sep_str;
				break;
			case ENGINE_JURABIB:
				label += before_str + 
					wrapCitation(*it, author + ' ' + year, for_xhtml) + sep_str;
				break;
			case ENGINE_BASIC:
				break;
			}

		
		} 
		// author; author; ...
		else if (cite_type == "citeauthor") {
			label += wrapCitation(*it, author, for_xhtml) + sep_str;
		}
		// year; year; ...
		else if (cite_type == "citeyear" ||
			   cite_type == "citeyearpar") {
			label += wrapCitation(*it, year, for_xhtml) + sep_str;
		}
	}
	label = rtrim(rtrim(label), sep);

	if (!after_str.empty()) {
		if (cite_type == "citet") {
			// insert "after" before last ')'
			label.insert(label.size() - 1, after_str);
		} else {
			bool const add =
				!(engine == ENGINE_NATBIB_NUMERICAL &&
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

	if (cite_type == "citep" || cite_type == "citeyearpar" || 
	    (cite_type == "cite" && engine == ENGINE_BASIC) )
		label = op + label + cp;

	return label;
}


docstring InsetCitation::basicLabel(bool for_xhtml) const
{
	docstring keys = getParam("key");
	docstring label;

	docstring key;
	do {
		// if there is no comma, then everything goes into key
		// and keys will be empty.
		keys = trim(split(keys, key, ','));
		key = trim(key);
		if (!label.empty())
			label += ", ";
		label += wrapCitation(key, key, for_xhtml);
	} while (!keys.empty());

	docstring const & after = getParam("after");
	if (!after.empty())
		label += ", " + after;

	return '[' + label + ']';
}

docstring InsetCitation::screenLabel() const
{
	return cache.screen_label;
}


void InsetCitation::updateBuffer(ParIterator const &, UpdateType)
{
	if (!cache.recalculate && buffer().citeLabelsValid())
		return;

	// The label may have changed, so we have to re-create it.
	docstring const glabel = generateLabel();

	unsigned int const maxLabelChars = 45;

	docstring label = glabel;
	if (label.size() > maxLabelChars) {
		label.erase(maxLabelChars - 3);
		label += "...";
	}

	cache.recalculate = false;
	cache.generated_label = glabel;
	cache.screen_label = label;
}


void InsetCitation::addToToc(DocIterator const & cpit) const
{
	// NOTE
	// XHTML output uses the TOC to collect the citations
	// from the document. So if this gets changed, then we
	// will need to change how the citations are collected.
	docstring const tocitem = getParam("key");
	Toc & toc = buffer().tocBackend().toc("citation");
	toc.push_back(TocItem(cpit, 0, tocitem));
}


int InsetCitation::plaintext(odocstream & os, OutputParams const &) const
{
	os << cache.generated_label;
	return cache.generated_label.size();
}


static docstring const cleanupWhitespace(docstring const & citelist)
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


int InsetCitation::docbook(odocstream & os, OutputParams const &) const
{
	os << from_ascii("<citation>")
	   << cleanupWhitespace(getParam("key"))
	   << from_ascii("</citation>");
	return 0;
}


docstring InsetCitation::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	string const & cmd = getCmdName();
	if (cmd == "nocite")
		return docstring();

	// have to output this raw, because generateLabel() will include tags
	xs << XHTMLStream::ESCAPE_NONE << generateLabel(true);

	return docstring();
}


void InsetCitation::toString(odocstream & os) const
{
	plaintext(os, OutputParams(0));
}


void InsetCitation::forToc(docstring & os, size_t) const
{
	os += screenLabel();
}


// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to \cite[]{}
void InsetCitation::latex(otexstream & os, OutputParams const & runparams) const
{
	CiteEngine cite_engine = buffer().params().citeEngine();
	BiblioInfo const & bi = buffer().masterBibInfo();
	// FIXME UNICODE
	docstring const cite_str = from_utf8(
		asValidLatexCommand(getCmdName(), cite_engine));

	if (runparams.inulemcmd)
		os << "\\mbox{";

	os << "\\" << cite_str;

	docstring const & before = getParam("before");
	docstring const & after  = getParam("after");
	if (!before.empty() && cite_engine != ENGINE_BASIC)
		os << '[' << before << "][" << after << ']';
	else if (!after.empty())
		os << '[' << after << ']';

	if (!bi.isBibtex(getParam("key")))
		// escape chars with bibitems
		os << '{' << escape(cleanupWhitespace(getParam("key"))) << '}';
	else
		os << '{' << cleanupWhitespace(getParam("key")) << '}';

	if (runparams.inulemcmd)
		os << "}";
}


void InsetCitation::validate(LaTeXFeatures & features) const
{
	switch (features.bufferParams().citeEngine()) {
	case ENGINE_BASIC:
		break;
	case ENGINE_NATBIB_AUTHORYEAR:
	case ENGINE_NATBIB_NUMERICAL:
		features.require("natbib");
		break;
	case ENGINE_JURABIB:
		features.require("jurabib");
		break;
	}
}


docstring InsetCitation::contextMenuName() const
{
	return from_ascii("context-citation");
}


} // namespace lyx
