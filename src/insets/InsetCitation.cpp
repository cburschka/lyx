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
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "output_xhtml.h"
#include "output_docbook.h"
#include "ParIterator.h"
#include "texstream.h"
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

InsetCitation::InsetCitation(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{
	buffer().removeBiblioTempFiles();
}


InsetCitation::~InsetCitation()
{
	if (isBufferLoaded())
		/* We do not use buffer() because Coverity believes that this
		 * may throw an exception. Actually this code path is not
		 * taken when buffer_ == 0 */
		buffer_->removeBiblioTempFiles();
}


// May well be over-ridden when session settings are loaded
// in GuiCitation. Unfortunately, that will not happen until
// such a dialog is created.
bool InsetCitation::last_literal = true;


ParamInfo const & InsetCitation::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;

	// standard cite does only take one argument, but biblatex, jurabib
	// and natbib extend this to two arguments, so
	// we have to allow both here. InsetCitation takes care that
	// LaTeX output is nevertheless correct.
	if (param_info_.empty()) {
		param_info_.add("after", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("before", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("key", ParamInfo::LATEX_REQUIRED);
		param_info_.add("pretextlist", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("posttextlist", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("literal", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


// We allow any command here, since we fall back to cite
// anyway if a command is not allowed by a style
bool InsetCitation::isCompatibleCommand(string const &)
{
	return true;
}


CitationStyle InsetCitation::getCitationStyle(BufferParams const & bp, string const & input,
				  vector<CitationStyle> const & valid_styles) const
{
	CitationStyle cs = valid_styles[0];
	cs.forceUpperCase = false;
	cs.hasStarredVersion = false;

	string normalized_input = input;
	string::size_type const n = input.size() - 1;
	if (isUpperCase(input[0]))
		normalized_input[0] = lowercase(input[0]);
	if (input[n] == '*')
		normalized_input = normalized_input.substr(0, n);

	string const alias = bp.getCiteAlias(normalized_input);
	if (!alias.empty())
		normalized_input = alias;

	vector<CitationStyle>::const_iterator it  = valid_styles.begin();
	vector<CitationStyle>::const_iterator end = valid_styles.end();
	for (; it != end; ++it) {
		CitationStyle this_cs = *it;
		if (this_cs.name == normalized_input) {
			cs = *it;
			break;
		}
	}

	return cs;
}


void InsetCitation::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY: {
		buffer().removeBiblioTempFiles();
		cache.recalculate = true;
		if (cmd.getArg(0) == "toggleparam") {
			string cmdname = getCmdName();
			string const alias =
				buffer().masterParams().getCiteAlias(cmdname);
			if (!alias.empty())
				cmdname = alias;
			string const par = cmd.getArg(1);
			string newcmdname = cmdname;
			if (par == "star") {
				if (suffixIs(cmdname, "*"))
					newcmdname = rtrim(cmdname, "*");
				else
					newcmdname = cmdname + "*";
			} else if (par == "casing") {
				if (isUpperCase(cmdname[0]))
					newcmdname[0] = lowercase(cmdname[0]);
				else
					newcmdname[0] = uppercase(newcmdname[0]);
			}
			cmd = FuncRequest(LFUN_INSET_MODIFY, "changetype " + newcmdname);
		}
	}
	// fall through
	default:
		InsetCommand::doDispatch(cur, cmd);
	}
}


bool InsetCitation::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	// Handle the alias case
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			string cmdname = getCmdName();
			string const alias =
				buffer().masterParams().getCiteAlias(cmdname);
			if (!alias.empty())
				cmdname = alias;
			if (suffixIs(cmdname, "*"))
				cmdname = rtrim(cmdname, "*");
			string const newtype = cmd.getArg(1);
			status.setEnabled(isCompatibleCommand(newtype));
			status.setOnOff(newtype == cmdname);
		}
		if (cmd.getArg(0) == "toggleparam") {
			string cmdname = getCmdName();
			string const alias =
				buffer().masterParams().getCiteAlias(cmdname);
			if (!alias.empty())
				cmdname = alias;
			vector<CitationStyle> citation_styles =
				buffer().masterParams().citeStyles();
			CitationStyle cs = getCitationStyle(buffer().masterParams(),
							    cmdname, citation_styles);
			if (cmd.getArg(1) == "star") {
				status.setEnabled(cs.hasStarredVersion);
				status.setOnOff(suffixIs(cmdname, "*"));
			}
			else if (cmd.getArg(1) == "casing") {
				status.setEnabled(cs.forceUpperCase);
				status.setOnOff(isUpperCase(cmdname[0]));
			}
		}
		return true;
	default:
		return InsetCommand::getStatus(cur, cmd, status);
	}
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

	CiteItem ci;
	ci.richtext = true;
	vector<docstring> keys = getVectorFromString(key);
	if (keys.size() == 1)
		return bi.getInfo(keys[0], buffer(), ci);

	docstring tip;
	tip += "<ol>";
	int count = 0;
	for (docstring const & kvar : keys) {
		docstring const key_info = bi.getInfo(kvar, buffer(), ci);
		// limit to reasonable size.
		if (count > 9 && keys.size() > 11) {
			tip.push_back(0x2026);// HORIZONTAL ELLIPSIS
			tip += "<p>"
				+ bformat(_("+ %1$d more entries."), int(keys.size() - count))
				+ "</p>";
			break;
		}
		if (key_info.empty())
			continue;
		tip += "<li>" + key_info + "</li>";
		++count;
	}
	tip += "</ol>";
	return tip;
}


namespace {

CitationStyle asValidLatexCommand(BufferParams const & bp, string const & input,
				  vector<CitationStyle> const & valid_styles)
{
	CitationStyle cs = valid_styles[0];
	cs.forceUpperCase = false;
	cs.hasStarredVersion = false;

	string normalized_input = input;
	string::size_type const n = input.size() - 1;
	if (isUpperCase(input[0]))
		normalized_input[0] = lowercase(input[0]);
	if (input[n] == '*')
		normalized_input = normalized_input.substr(0, n);

	string const alias = bp.getCiteAlias(normalized_input);
	if (!alias.empty())
		normalized_input = alias;

	vector<CitationStyle>::const_iterator it  = valid_styles.begin();
	vector<CitationStyle>::const_iterator end = valid_styles.end();
	for (; it != end; ++it) {
		CitationStyle this_cs = *it;
		if (this_cs.name == normalized_input) {
			cs = *it;
			break;
		}
	}

	cs.forceUpperCase &= input[0] == uppercase(input[0]);
	cs.hasStarredVersion &= input[n] == '*';

	return cs;
}


inline docstring wrapCitation(docstring const & key,
		docstring const & content, bool for_xhtml)
{
	if (!for_xhtml)
		return content;
	// we have to do the escaping here, because we will ultimately
	// write this as a raw string, so as not to escape the tags.
	return "<a href='#LyXCite-" + xml::cleanAttr(key) + "'>" +
			xml::escapeString(content, XMLStream::ESCAPE_ALL) + "</a>";
}

} // anonymous namespace


vector<pair<docstring, docstring>> InsetCitation::getQualifiedLists(docstring const & p) const
{
	vector<docstring> ps =
		getVectorFromString(p, from_ascii("\t"));
	QualifiedList res;
	for (docstring const & s: ps) {
		docstring key = s;
		docstring val;
		if (contains(s, ' '))
			val = split(s, key, ' ');
		res.push_back(make_pair(key, val));
	}
	return res;
}

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

	docstring const & key = getParam("key");

	BiblioInfo const & biblist = buf.masterBibInfo();

	// mark broken citations
	setBroken(false);

	if (biblist.empty()) {
		setBroken(true);
		return docstring();
	}

	if (key.empty())
		return _("No citations selected!");

	// check all citations
	// we only really want the last 'false', to suppress trimming, but
	// we need to give the other defaults, too, to set it.
	vector<docstring> keys =
		getVectorFromString(key, from_ascii(","), false, false);
	for (auto const & k : keys) {
		if (biblist.find(k) == biblist.end()) {
			setBroken(true);
			break;
		}
	}
	
	string cite_type = getCmdName();
	bool const uppercase = isUpperCase(cite_type[0]);
	if (uppercase)
		cite_type[0] = lowercase(cite_type[0]);
	bool const starred = (cite_type[cite_type.size() - 1] == '*');
	if (starred)
		cite_type = cite_type.substr(0, cite_type.size() - 1);

	// handle alias
	string const alias = buf.masterParams().getCiteAlias(cite_type);
	if (!alias.empty())
		cite_type = alias;

	// FIXME: allow to add cite macros
	/*
	buffer().params().documentClass().addCiteMacro("!textbefore", to_utf8(before));
	buffer().params().documentClass().addCiteMacro("!textafter", to_utf8(after));
	*/
	docstring label;
	CitationStyle cs = getCitationStyle(buffer().masterParams(),
			cite_type, buffer().masterParams().citeStyles());
	bool const qualified = cs.hasQualifiedList
		&& (keys.size() > 1
		    || !getParam("pretextlist").empty()
		    || !getParam("posttextlist").empty());
	QualifiedList pres = getQualifiedLists(getParam("pretextlist"));
	QualifiedList posts = getQualifiedLists(getParam("posttextlist"));

	CiteItem ci;
	ci.textBefore = getParam("before");
	ci.textAfter = getParam("after");
	ci.forceUpperCase = uppercase;
	ci.Starred = starred;
	ci.max_size = UINT_MAX;
	ci.isQualified = qualified;
	ci.pretexts = pres;
	ci.posttexts = posts;
	if (for_xhtml) {
		ci.max_key_size = UINT_MAX;
		ci.context = CiteItem::Export;
	}
	ci.richtext = for_xhtml;
	label = biblist.getLabel(keys, buffer(), cite_type, ci);
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
		keys = split(keys, key, ',');
		if (!label.empty())
			label += ", ";
		label += wrapCitation(key, key, for_xhtml);
	} while (!keys.empty());

	docstring const & after = getParam("after");
	if (!after.empty())
		label += ", " + after;

	return '[' + label + ']';
}


bool InsetCitation::forceLTR(OutputParams const & rp) const
{
	// We have to force LTR for numeric references
	// [= bibliography, plain BibTeX, numeric natbib
	// and biblatex]. Except for XeTeX/bidi. See #3005.
	if (rp.useBidiPackage())
		return false;
	return (buffer().masterParams().citeEngine() == "basic"
		|| buffer().masterParams().citeEngineType() == ENGINE_TYPE_NUMERICAL);
}

docstring InsetCitation::screenLabel() const
{
	return cache.screen_label;
}


void InsetCitation::updateBuffer(ParIterator const &, UpdateType, bool const /*deleted*/)
{
	if (!cache.recalculate && buffer().citeLabelsValid())
		return;
	// The label may have changed, so we have to re-create it.
	docstring const glabel = generateLabel();
	cache.recalculate = false;
	cache.generated_label = glabel;
	unsigned int const maxLabelChars = 45;
	cache.screen_label = glabel;
	support::truncateWithEllipsis(cache.screen_label, maxLabelChars, true);
}


void InsetCitation::addToToc(DocIterator const & cpit, bool output_active,
							 UpdateType, TocBackend & backend) const
{
	// NOTE
	// BiblioInfo::collectCitedEntries() uses the TOC to collect the citations
	// from the document. It is used indirectly, via BiblioInfo::makeCitationLables,
	// by both XHTML and plaintext output. So, if we change what goes into the TOC,
	// then we will also need to change that routine.
	docstring tocitem;
	if (isBroken())
		tocitem = _("BROKEN: ");
	tocitem += getParam("key");
	TocBuilder & b = backend.builder("citation");
	b.pushItem(cpit, tocitem, output_active);
	b.pop();
	if (isBroken()) {
		shared_ptr<Toc> toc2 = backend.toc("brokenrefs");
		toc2->push_back(TocItem(cpit, 0, tocitem, output_active));
	}
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


void InsetCitation::docbook(XMLStream & xs, OutputParams const &) const
{
	if (getCmdName() == "nocite")
		return;

	// Split the different citations (on ","), so that one tag can be output for each of them.
	// DocBook does not support having multiple citations in one tag, so that we have to deal with formatting here.
	docstring citations = getParam("key");
	if (citations.find(',') == string::npos) {
		xs << xml::CompTag("biblioref", "endterm=\"" + to_utf8(xml::cleanID(citations)) + "\"");
	} else {
		size_t pos = 0;
		while (pos != string::npos) {
			pos = citations.find(',');
			xs << xml::CompTag("biblioref", "endterm=\"" + to_utf8(xml::cleanID(citations.substr(0, pos))) + "\"");
			citations.erase(0, pos + 1);

			if (pos != string::npos) {
				xs << ", "; 
			}
		}
	}
}


docstring InsetCitation::xhtml(XMLStream & xs, OutputParams const &) const
{
	if (getCmdName() == "nocite")
		return docstring();

	// have to output this raw, because generateLabel() will include tags
	xs << XMLStream::ESCAPE_NONE << generateLabel(true);

	return docstring();
}


void InsetCitation::toString(odocstream & os) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os << ods.str();
}


void InsetCitation::forOutliner(docstring & os, size_t const, bool const) const
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
	// When this is a child compiled on its own, we use the children
	// own bibinfo, else the master's
	BiblioInfo const & bi = runparams.is_child
			? buffer().masterBibInfo() : buffer().bibInfo();
	docstring const key = getParam("key");
	// "keyonly" command: output the plain key and stop.
	if (getCmdName() == "keyonly") {
		// Special command to only return the key
		if (!bi.isBibtex(getParam("key")))
			// escape chars with bibitems
			os << escape(cleanupWhitespace(key));
		else
			os << cleanupWhitespace(key);
		return;
	}
	vector<CitationStyle> citation_styles = buffer().masterParams().citeStyles();
	CitationStyle cs = asValidLatexCommand(buffer().masterParams(),
					       getCmdName(), citation_styles);
	// FIXME UNICODE
	docstring const cite_str = from_utf8(citationStyleToString(cs, true));

	// check if we have to do a qualified list
	vector<docstring> keys = getVectorFromString(cleanupWhitespace(key));
	bool const qualified = cs.hasQualifiedList
		&& (!getParam("pretextlist").empty()
		    || !getParam("posttextlist").empty());

	if (runparams.inulemcmd > 0)
		os << "\\mbox{";

	os << "\\" << cite_str;

	if (qualified)
		os << "s";

	ParamInfo const & pinfo = findInfo(string());
	docstring before = params().prepareCommand(runparams, getParam("before"),
						   pinfo["before"].handling());
	docstring after = params().prepareCommand(runparams, getParam("after"),
						   pinfo["after"].handling());
	if (!before.empty() && cs.textBefore) {
		if (qualified)
			os << '(' << protectArgument(before, '(', ')')
			   << ")(" << protectArgument(after, '(', ')') << ')';
		else
			os << '[' << protectArgument(before) << "]["
			   << protectArgument(after) << ']';
	} else if (!after.empty() && cs.textAfter) {
		if (qualified)
			os << '(' << protectArgument(after, '(', ')') << ')';
		else
			os << '[' << protectArgument(after) << ']';
	}

	if (!bi.isBibtex(key))
		// escape chars with bibitems
		os << '{' << escape(cleanupWhitespace(key)) << '}';
	else {
		if (qualified) {
			QualifiedList pres = getQualifiedLists(getParam("pretextlist"));
			QualifiedList posts = getQualifiedLists(getParam("posttextlist"));
			for (docstring const & k : keys) {
				docstring prenote;
				QualifiedList::iterator it = pres.begin();
				for (; it != pres.end() ; ++it) {
					if ((*it).first == k) {
						prenote = (*it).second;
						pres.erase(it);
						break;
					}
				}
				docstring bef = params().prepareCommand(runparams, prenote,
				                   pinfo["pretextlist"].handling());
				docstring postnote;
				QualifiedList::iterator pit = posts.begin();
				for (; pit != posts.end() ; ++pit) {
					if ((*pit).first == k) {
						postnote = (*pit).second;
						posts.erase(pit);
						break;
					}
				}
				docstring aft = params().prepareCommand(runparams, postnote,
				                   pinfo["posttextlist"].handling());
				if (!bef.empty())
					os << '[' << protectArgument(bef)
					   << "][" << protectArgument(aft) << ']';
				else if (!aft.empty())
					os << '[' << protectArgument(aft) << ']';
				os << '{' << k << '}';
			}
		} else
			os << '{' << cleanupWhitespace(key) << '}';
	}

	if (runparams.inulemcmd)
		os << "}";
}


string InsetCitation::contextMenuName() const
{
	return "context-citation";
}


} // namespace lyx
