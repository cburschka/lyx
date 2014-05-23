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
#include <climits>

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


// FIXME: use the citeCommands provided by the TextClass
// instead of possibleCiteCommands defined in this file.
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


bool InsetCitation::addKey(string const & key)
{
	docstring const ukey = from_utf8(key);
	docstring const & curkeys = getParam("key");
	if (curkeys.empty()) {
		setParam("key", ukey);
		cache.recalculate = true;
		return true;
	}

	vector<docstring> keys = getVectorFromString(curkeys);
	vector<docstring>::const_iterator it = keys.begin();
	vector<docstring>::const_iterator en = keys.end();
	for (; it != en; ++it) {
		if (*it == ukey) {
			LYXERR0("Key " << key << " already present.");
			return false;
		}
	}
	keys.push_back(ukey);
	setParam("key", getStringFromVector(keys));
	cache.recalculate = true;
	return true;
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


CitationStyle asValidLatexCommand(string const & input, vector<CitationStyle> const valid_styles)
{
	CitationStyle cs = valid_styles[0];
	cs.forceUpperCase = false;
	cs.fullAuthorList = false;
	if (!InsetCitation::isCompatibleCommand(input))
		return cs;

	string normalized_input = input;
	string::size_type const n = input.size() - 1;
	if (input[0] == 'C')
		normalized_input[0] = 'c';
	if (input[n] == '*')
		normalized_input = normalized_input.substr(0, n);

	vector<CitationStyle>::const_iterator it  = valid_styles.begin();
	vector<CitationStyle>::const_iterator end = valid_styles.end();
	for (; it != end; ++it) {
		CitationStyle this_cs = *it;
		if (this_cs.cmd == normalized_input) {
			cs = *it;
			break;
		}
	}

	cs.forceUpperCase &= input[0] == 'C';
	cs.fullAuthorList &= input[n] == '*';

	return cs;
}


inline docstring wrapCitation(docstring const & key,
		docstring const & content, bool for_xhtml)
{
	if (!for_xhtml)
		return content;
	// we have to do the escaping here, because we will ultimately
	// write this as a raw string, so as not to escape the tags.
	return "<a href='#LyXCite-" + html::cleanAttr(key) + "'>" +
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

	docstring const & key = getParam("key");
	if (key.empty())
		return _("No citations selected!");

	// We don't currently use the full or forceUCase fields.
	string cite_type = getCmdName();
	if (cite_type[0] == 'C')
		// If we were going to use them, this would mean ForceUCase
		cite_type = string(1, 'c') + cite_type.substr(1);
	if (cite_type[cite_type.size() - 1] == '*')
		// and this would mean FULL
		cite_type = cite_type.substr(0, cite_type.size() - 1);

	docstring const & before = getParam("before");
	docstring const & after = getParam("after");

	// FIXME: allow to add cite macros
	/*
	buffer().params().documentClass().addCiteMacro("!textbefore", to_utf8(before));
	buffer().params().documentClass().addCiteMacro("!textafter", to_utf8(after));
	*/
	docstring label;
	vector<docstring> keys = getVectorFromString(key);
	label = biblist.getLabel(keys, buffer(), cite_type, for_xhtml, UINT_MAX, before, after);
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


void InsetCitation::addToToc(DocIterator const & cpit, bool output_active) const
{
	// NOTE
	// XHTML output uses the TOC to collect the citations
	// from the document. So if this gets changed, then we
	// will need to change how the citations are collected.
	docstring const tocitem = getParam("key");
	Toc & toc = buffer().tocBackend().toc("citation");
	toc.push_back(TocItem(cpit, 0, tocitem, output_active));
}


int InsetCitation::plaintext(odocstringstream & os,
       OutputParams const &, size_t) const
{
	string const & cmd = getCmdName();
	if (cmd == "nocite")
		return 0;

	docstring const label = generateLabel(false);
	os << label;
	return label.size();
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
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os << ods.str();
}


void InsetCitation::forToc(docstring & os, size_t) const
{
	os += screenLabel();
}


// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to the default citation command as provided by the citation
// engine, e.g. \cite[]{} for the basic engine.
void InsetCitation::latex(otexstream & os, OutputParams const & runparams) const
{
	vector<CitationStyle> citation_styles = buffer().params().citeStyles();
	CitationStyle cs = asValidLatexCommand(getCmdName(), citation_styles);
	BiblioInfo const & bi = buffer().masterBibInfo();
	// FIXME UNICODE
	docstring const cite_str = from_utf8(citationStyleToString(cs));

	if (runparams.inulemcmd > 0)
		os << "\\mbox{";

	os << "\\" << cite_str;

	docstring const & before = getParam("before");
	docstring const & after  = getParam("after");
	if (!before.empty() && cs.textBefore)
		os << '[' << before << "][" << after << ']';
	else if (!after.empty() && cs.textAfter)
		os << '[' << after << ']';

	if (!bi.isBibtex(getParam("key")))
		// escape chars with bibitems
		os << '{' << escape(cleanupWhitespace(getParam("key"))) << '}';
	else
		os << '{' << cleanupWhitespace(getParam("key")) << '}';

	if (runparams.inulemcmd)
		os << "}";
}


string InsetCitation::contextMenuName() const
{
	return "context-citation";
}


} // namespace lyx
