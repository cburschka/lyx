/**
 * \file InsetIndex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetIndex.h"
#include "InsetIndexMacro.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "ColorSet.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "IndicesList.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeX.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyX.h"
#include "output_latex.h"
#include "output_xhtml.h"
#include "xml.h"
#include "texstream.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Translator.h"

#include "frontends/alert.h"

#include <algorithm>
#include <set>
#include <iostream>

#include <QThreadStorage>

using namespace std;
using namespace lyx::support;

// Uncomment to enable InsetIndex-specific debugging mode: the tree for the index will be printed to std::cout.
// #define LYX_INSET_INDEX_DEBUG

namespace lyx {

namespace {

typedef Translator<string, InsetIndexParams::PageRange> PageRangeTranslator;
typedef Translator<docstring, InsetIndexParams::PageRange> PageRangeTranslatorLoc;

PageRangeTranslator const init_insetindexpagerangetranslator()
{
	PageRangeTranslator translator("none", InsetIndexParams::None);
	translator.addPair("start", InsetIndexParams::Start);
	translator.addPair("end", InsetIndexParams::End);
	return translator;
}

PageRangeTranslator const init_insetindexpagerangetranslator_latex()
{
	PageRangeTranslator translator("", InsetIndexParams::None);
	translator.addPair("(", InsetIndexParams::Start);
	translator.addPair(")", InsetIndexParams::End);
	return translator;
}


PageRangeTranslatorLoc const init_insetindexpagerangetranslator_loc()
{
	PageRangeTranslatorLoc translator(docstring(), InsetIndexParams::None);
	translator.addPair(_("Starts page range"), InsetIndexParams::Start);
	translator.addPair(_("Ends page range"), InsetIndexParams::End);
	return translator;
}


PageRangeTranslator const & insetindexpagerangetranslator()
{
	static PageRangeTranslator const prtranslator =
			init_insetindexpagerangetranslator();
	return prtranslator;
}


PageRangeTranslatorLoc const & insetindexpagerangetranslator_loc()
{
	static PageRangeTranslatorLoc const translator =
			init_insetindexpagerangetranslator_loc();
	return translator;
}


PageRangeTranslator const & insetindexpagerangetranslator_latex()
{
	static PageRangeTranslator const lttranslator =
			init_insetindexpagerangetranslator_latex();
	return lttranslator;
}

} // namespace anon

/////////////////////////////////////////////////////////////////////
//
// InsetIndex
//
///////////////////////////////////////////////////////////////////////


InsetIndex::InsetIndex(Buffer * buf, InsetIndexParams const & params)
	: InsetCollapsible(buf), params_(params)
{}


void InsetIndex::latex(otexstream & ios, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	runparams.inIndexEntry = true;
	if (runparams_in.postpone_fragile_stuff)
		// This is not needed and would impact sorting
		runparams.moving_arg = false;

	otexstringstream os;

	if (buffer().masterBuffer()->params().use_indices && !params_.index.empty()
		&& params_.index != "idx") {
		os << "\\sindex[";
		os << escape(params_.index);
		os << "]{";
	} else {
		os << "\\index";
		os << '{';
	}

	// Get the LaTeX output from InsetText. We need to deconstruct this later
	// in order to check if we need to generate a sorting key
	odocstringstream ourlatex;
	otexstream ots(ourlatex);
	InsetText::latex(ots, runparams);
	if (runparams.find_effective()) {
		// No need for special handling, if we are only searching for some patterns
		os << ourlatex.str() << "}";
		return;
	}

	if (hasSortKey()) {
		getSortkey(os, runparams);
		os << "@";
		os << ourlatex.str();
		getSubentries(os, runparams, ourlatex.str());
		if (hasSeeRef()) {
			os << "|";
			os << insetindexpagerangetranslator_latex().find(params_.range);
			getSeeRefs(os, runparams);
		} else if (!params_.pagefmt.empty() && params_.pagefmt != "default") {
			os << "|";
			os << insetindexpagerangetranslator_latex().find(params_.range);
			os << from_utf8(params_.pagefmt);
		} else if (params_.range != InsetIndexParams::PageRange::None) {
			os << "|";
			os << insetindexpagerangetranslator_latex().find(params_.range);
		}
	} else {
		// We check whether we need a sort key.
		// If so, we use the plaintext version
		odocstringstream ourplain;
		InsetText::plaintext(ourplain, runparams);

		// These are the LaTeX and plaintext representations
		docstring latexstr = ourlatex.str();
		docstring plainstr = ourplain.str();
	
		// This will get what follows | if anything does,
		// the command (e.g., see, textbf) for pagination
		// formatting
		docstring cmd;

		if (hasSeeRef()) {
			odocstringstream seeref;
			otexstream otsee(seeref);
			getSeeRefs(otsee, runparams);
			cmd = seeref.str();
		} else if (!params_.pagefmt.empty() && params_.pagefmt != "default") {
			cmd = from_utf8(params_.pagefmt);
		} else {
			// Check for the | separator to strip the cmd.
			// This goes wrong on an escaped "|", but as the escape
			// character can be changed in style files, we cannot
			// prevent that.
			size_t pos = latexstr.find(from_ascii("|"));
			if (pos != docstring::npos) {
				// Put the bit after "|" into cmd...
				cmd = latexstr.substr(pos + 1);
				// ...and erase that stuff from latexstr
				latexstr = latexstr.erase(pos);
				// ...as well as from plainstr
				size_t ppos = plainstr.find(from_ascii("|"));
				if (ppos < plainstr.size())
					plainstr.erase(ppos);
				else
					LYXERR0("The `|' separator was not found in the plaintext version!");
			}
		}

		odocstringstream subentries;
		otexstream otsub(subentries);
		getSubentries(otsub, runparams, ourlatex.str());
		if (subentries.str().empty()) {
			// Separate the entries and subentries, i.e., split on "!".
			// This goes wrong on an escaped "!", but as the escape
			// character can be changed in style files, we cannot
			// prevent that.
			std::vector<docstring> const levels =
					getVectorFromString(latexstr, from_ascii("!"), true);
			std::vector<docstring> const levels_plain =
					getVectorFromString(plainstr, from_ascii("!"), true);
		
			vector<docstring>::const_iterator it = levels.begin();
			vector<docstring>::const_iterator end = levels.end();
			vector<docstring>::const_iterator it2 = levels_plain.begin();
			bool first = true;
			for (; it != end; ++it) {
				if ((*it).empty()) {
					emptySubentriesWarning(ourlatex.str());
					if (it2 < levels_plain.end())
						++it2;
					continue;
				}
				// The separator needs to be put back when
				// writing the levels, except for the first level
				if (!first)
					os << '!';
				else
					first = false;
		
				// Now here comes the reason for this whole procedure:
				// We try to correctly sort macros and formatted strings.
				// If we find a command, prepend a plain text
				// version of the content to get sorting right,
				// e.g. \index{LyX@\LyX}, \index{text@\textbf{text}}.
				// We do this on all levels.
				// We don't do it if the level already contains a '@', though.
				// Plaintext might return nothing (e.g. for ERTs).
				// In that case, we use LaTeX.
				docstring const spart = (levels_plain.empty() || (*it2).empty()) ? *it : *it2;
				processLatexSorting(os, runparams, *it, spart);
				if (it2 < levels_plain.end())
					++it2;
			}
		} else {
			processLatexSorting(os, runparams, latexstr, plainstr);
			os << subentries.str();
		}

		// At last, re-insert the command, separated by "|"
		if (!cmd.empty()) {
			os << "|"
			   << insetindexpagerangetranslator_latex().find(params_.range)
			   << cmd;
		} else if (params_.range != InsetIndexParams::PageRange::None) {
			os << "|";
			os << insetindexpagerangetranslator_latex().find(params_.range);
		}
	}
	os << '}';

	// In macros with moving arguments, such as \section,
	// we store the index and output it after the macro (#2154)
	if (runparams_in.postpone_fragile_stuff)
		runparams_in.post_macro += os.str();
	else
		ios << os.release();
}


void InsetIndex::processLatexSorting(otexstream & os, OutputParams const & runparams,
				docstring const latex, docstring const spart) const
{
	if (contains(latex, '\\') && !contains(latex, '@')) {
		// Now we need to validate that all characters in
		// the sorting part are representable in the current
		// encoding. If not try the LaTeX macro which might
		// or might not be a good choice, and issue a warning.
		pair<docstring, docstring> spart_latexed =
				runparams.encoding->latexString(spart, runparams.dryrun);
		if (!spart_latexed.second.empty())
			LYXERR0("Uncodable character in index entry. Sorting might be wrong!");
		if (spart != spart_latexed.first && !runparams.dryrun) {
			TeXErrors terr;
			ErrorList & errorList = buffer().errorList("Export");
			docstring const s = bformat(_("LyX's automatic index sorting algorithm faced "
						      "problems with the entry '%1$s'.\n"
						      "Please specify the sorting of this entry manually, as "
						      "explained in the User Guide."), spart);
			Paragraph const & par = buffer().paragraphs().front();
			errorList.push_back(ErrorItem(_("Index sorting failed"), s,
						      {par.id(), 0}, {par.id(), -1}));
			buffer().bufferErrors(terr, errorList);
		}
		// Remove remaining \'s from the sort key
		docstring ppart = subst(spart_latexed.first, from_ascii("\\"), docstring());
		// Plain quotes need to be escaped, however (#10649), as this
		// is the default escape character
		ppart = subst(ppart, from_ascii("\""), from_ascii("\\\""));

		// Now insert the sortkey, separated by '@'.
		os << ppart;
		os << '@';
	}
	// Insert the actual level text
	os << latex;
}


void InsetIndex::docbook(XMLStream & xs, OutputParams const & runparams) const
{
	// Two ways of processing this inset are implemented:
	// - the legacy one, based on parsing the raw LaTeX (before LyX 2.4) -- unlikely to be deprecated
	// - the modern one, based on precise insets for indexing features
	// Like the LaTeX implementation, consider the user chooses either of those options.

	// Get the content of the inset as LaTeX, as some things may be encoded as ERT (like {}).
	// TODO: if there is an ERT within the index term, its conversion should be tried, in case it becomes useful;
	//  otherwise, ERTs should become comments. For now, they are just copied as-is, which is barely satisfactory.
	odocstringstream odss;
	otexstream ots(odss);
	InsetText::latex(ots, runparams);
	docstring latexString = trim(odss.str());

	// Handle several indices (indicated in the inset instead of the raw latexString).
	docstring indexType = from_utf8("");
	if (buffer().masterBuffer()->params().use_indices) {
		indexType += " type=\"" + params_.index + "\"";
	}

	// Split the string into its main constituents: terms, and command (see, see also, range).
	size_t positionVerticalBar = latexString.find(from_ascii("|")); // What comes before | is (sub)(sub)entries.
	docstring indexTerms = latexString.substr(0, positionVerticalBar);
	docstring command;
	if (positionVerticalBar != lyx::docstring::npos) {
		command = latexString.substr(positionVerticalBar + 1);
	}

	// Handle sorting issues, with @.
	docstring sortAs;
	if (hasSortKey()) {
		sortAs = getSortkeyAsText(runparams);
		// indexTerms may contain a sort key if the user has both the inset and the manual key.
	} else {
		vector<docstring> sortingElements = getVectorFromString(indexTerms, from_ascii("@"), false);
		if (sortingElements.size() == 2) {
			sortAs = sortingElements[0];
			indexTerms = sortingElements[1];
		}
	}

	// Handle primary, secondary, and tertiary terms (entries, subentries, and subsubentries, for LaTeX).
	vector<docstring> terms;
	if (const vector<docstring> potential_terms = getSubentriesAsText(runparams); !potential_terms.empty()) {
		terms = potential_terms;
		// The main term is not present in the vector, as it's not a subentry. The main index term is inserted raw in
		// the index inset. Considering that the user either uses the new or the legacy mechanism, the main term is the
		// full string within this inset (i.e. without the subinsets).
		terms.insert(terms.begin(), latexString);
	} else {
		terms = getVectorFromString(indexTerms, from_ascii("!"), false);
	}

	// Handle ranges. Happily, in the raw LaTeX mode, (| and |) can only be at the end of the string!
	const bool hasInsetRange = params_.range != InsetIndexParams::PageRange::None;
	const bool hasStartRange = params_.range == InsetIndexParams::PageRange::Start ||
			latexString.find(from_ascii("|(")) != lyx::docstring::npos;
	const bool hasEndRange = params_.range == InsetIndexParams::PageRange::End ||
			latexString.find(from_ascii("|)")) != lyx::docstring::npos;

	if (hasInsetRange) {
		// Remove the ranges from the command if they do not appear at the beginning.
		size_t index = 0;
		while ((index = command.find(from_utf8("|("), index)) != std::string::npos)
			command.erase(index, 1);
		index = 0;
		while ((index = command.find(from_utf8("|)"), index)) != std::string::npos)
			command.erase(index, 1);

		// Remove the ranges when they are the only vertical bar in the complete string.
		if (command[0] == '(' || command[0] == ')')
			command.erase(0, 1);
	}

	// Handle see and seealso. As "see" is a prefix of "seealso", the order of the comparisons is important.
	// Both commands are mutually exclusive!
	docstring see = getSeeAsText(runparams);
	vector<docstring> seeAlsoes = getSeeAlsoesAsText(runparams);

	if (see.empty() && seeAlsoes.empty() && command.substr(0, 3) == "see") {
		// Unescape brackets.
		size_t index = 0;
		while ((index = command.find(from_utf8("\\{"), index)) != std::string::npos)
			command.erase(index, 1);
		index = 0;
		while ((index = command.find(from_utf8("\\}"), index)) != std::string::npos)
			command.erase(index, 1);

		// Retrieve the part between brackets, and remove the complete seealso.
		size_t positionOpeningBracket = command.find(from_ascii("{"));
		size_t positionClosingBracket = command.find(from_ascii("}"));
		docstring list = command.substr(positionOpeningBracket + 1, positionClosingBracket - positionOpeningBracket - 1);

		// Parse the list of referenced entries (or a single one for see).
		if (command.substr(0, 7) == "seealso") {
			seeAlsoes = getVectorFromString(list, from_ascii(","), false);
		} else {
			see = list;

			if (see.find(from_ascii(",")) != std::string::npos) {
				docstring error = from_utf8("Several index terms found as \"see\"! Only one is acceptable. "
											"Complete entry: \"") + latexString + from_utf8("\"");
				LYXERR0(error);
				xs << XMLStream::ESCAPE_NONE << (from_utf8("<!-- Output Error: ") + error + from_utf8(" -->\n"));
			}
		}

		// Remove the complete see/seealso from the commands, in case there is something else to parse.
		command = command.substr(positionClosingBracket + 1);
	}

	// Some parts of the strings are not parsed, as they do not have anything matching in DocBook: things like
	// formatting the entry or the page number, other strings for sorting. https://wiki.lyx.org/Tips/Indexing
	// If there are such things in the index entry, then this code may miserably fail. For example, for "Peter|(textbf",
	// no range will be detected.
	// TODO: Could handle formatting as significance="preferred"?
	if (!command.empty()) {
		docstring error = from_utf8("Unsupported feature: an index entry contains a | with an unsupported command, ")
				          + command + from_utf8(". ") + from_utf8("Complete entry: \"") + latexString + from_utf8("\"");
		LYXERR0(error);
		xs << XMLStream::ESCAPE_NONE << (from_utf8("<!-- Output Error: ") + error + from_utf8(" -->\n"));
	}

	// Write all of this down.
	if (terms.empty() && !hasEndRange) {
		docstring error = from_utf8("No index term found! Complete entry: \"") + latexString + from_utf8("\"");
		LYXERR0(error);
		xs << XMLStream::ESCAPE_NONE << (from_utf8("<!-- Output Error: ") + error + from_utf8(" -->\n"));
	} else {
		// Generate the attributes for ranges. It is based on the terms that are indexed, but the ID must be unique
		// to this indexing area (xml::cleanID does not guarantee this: for each call with the same arguments,
		// the same legal ID is produced; here, as the input would be the same, the output must be, by design).
		// Hence the thread-local storage, as the numbers must strictly be unique, and thus cannot be shared across
		// a paragraph (making the solution used for HTML worthless). This solution is very similar to the one used in
		// xml::cleanID.
		// indexType can only be used for singular and startofrange types!
		docstring attrs;
		if (!hasStartRange && !hasEndRange) {
			attrs = indexType;
		} else {
			// Append an ID if uniqueness is not guaranteed across the document.
			static QThreadStorage<set<docstring>> tKnownTermLists;
			static QThreadStorage<int> tID;

			set<docstring> &knownTermLists = tKnownTermLists.localData();
			int &ID = tID.localData();

			if (!tID.hasLocalData()) {
				tID.localData() = 0;
			}

			// Modify the index terms to add the unique ID if needed.
			docstring newIndexTerms = indexTerms;
			if (knownTermLists.find(indexTerms) != knownTermLists.end()) {
				newIndexTerms += from_ascii(string("-") + to_string(ID));

				// Only increment for the end of range, so that the same number is used for the start of range.
				if (hasEndRange) {
					ID++;
				}
			}

			// Term list not yet known: add it to the set AFTER the end of range. After
			if (knownTermLists.find(indexTerms) == knownTermLists.end() && hasEndRange) {
				knownTermLists.insert(indexTerms);
			}

			// Generate the attributes.
			docstring id = xml::cleanID(newIndexTerms);
			if (hasStartRange) {
				attrs = indexType + " class=\"startofrange\" xml:id=\"" + id + "\"";
			} else {
				attrs = " class=\"endofrange\" startref=\"" + id + "\"";
			}
		}

		// Handle the index terms (including the specific index for this entry).
		if (hasEndRange) {
			xs << xml::CompTag("indexterm", attrs);
		} else {
			xs << xml::StartTag("indexterm", attrs);
			if (!terms.empty()) { // hasEndRange has no content.
				docstring attr;
				if (!sortAs.empty()) {
					attr = from_utf8("sortas='") + sortAs + from_utf8("'");
				}

				xs << xml::StartTag("primary", attr);
				xs << terms[0];
				xs << xml::EndTag("primary");
			}
			if (terms.size() > 1) {
				xs << xml::StartTag("secondary");
				xs << terms[1];
				xs << xml::EndTag("secondary");
			}
			if (terms.size() > 2) {
				xs << xml::StartTag("tertiary");
				xs << terms[2];
				xs << xml::EndTag("tertiary");
			}

			// Handle see and see also.
			if (!see.empty()) {
				xs << xml::StartTag("see");
				xs << see;
				xs << xml::EndTag("see");
			}

			if (!seeAlsoes.empty()) {
				for (auto &entry : seeAlsoes) {
					xs << xml::StartTag("seealso");
					xs << entry;
					xs << xml::EndTag("seealso");
				}
			}

			// Close the entry.
			xs << xml::EndTag("indexterm");
		}
	}
}


docstring InsetIndex::xhtml(XMLStream & xs, OutputParams const &) const
{
	// we just print an anchor, taking the paragraph ID from
	// our own interior paragraph, which doesn't get printed
	std::string const magic = paragraphs().front().magicLabel();
	std::string const attr = "id='" + magic + "'";
	xs << xml::CompTag("a", attr);
	return docstring();
}


bool InsetIndex::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("index", params2string(params_),
			const_cast<InsetIndex *>(this));
	return true;
}


void InsetIndex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) == "changetype") {
			cur.recordUndoInset(this);
			params_.index = from_utf8(cmd.getArg(1));
			break;
		}
		if (cmd.getArg(0) == "changeparam") {
			string const p = cmd.getArg(1);
			string const v = cmd.getArg(2);
			cur.recordUndoInset(this);
			if (p == "range")
				params_.range = insetindexpagerangetranslator().find(v);
			if (p == "pagefmt") {
				if (v == "default" || v == "textbf"
				    || v == "textit" || v == "emph")
					params_.pagefmt = v;
				else
					lyx::dispatch(FuncRequest(LFUN_INSET_SETTINGS, "index"));
			}
			break;
		}
		InsetIndexParams params;
		InsetIndex::string2params(to_utf8(cmd.argument()), params);
		cur.recordUndoInset(this);
		params_.index = params.index;
		params_.range = params.range;
		params_.pagefmt = params.pagefmt;
		// what we really want here is a TOC update, but that means
		// a full buffer update
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("index", params2string(params_));
		break;

	case LFUN_PARAGRAPH_BREAK: {
		// Since this inset in single-par anyway, let's use
		// return to enter subentries
		FuncRequest fr(LFUN_INDEXMACRO_INSERT, "subentry");
		lyx::dispatch(fr);
		break;
	}

	case LFUN_INSET_INSERT_COPY: {
		Cursor & bvcur = cur.bv().cursor();
		if (cmd.origin() == FuncRequest::TOC && bvcur.inTexted()) {
			cap::copyInsetToTemp(cur, clone());
			cap::pasteFromTemp(bvcur, bvcur.buffer()->errorList("Paste"));
		} else
			cur.undispatched();
		break;
	}

	default:
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}
}


bool InsetIndex::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			docstring const newtype = from_utf8(cmd.getArg(1));
			Buffer const & realbuffer = *buffer().masterBuffer();
			IndicesList const & indiceslist = realbuffer.params().indiceslist();
			Index const * index = indiceslist.findShortcut(newtype);
			flag.setEnabled(index != 0);
			flag.setOnOff(
				from_utf8(cmd.getArg(1)) == params_.index);
			return true;
		}
		if (cmd.getArg(0) == "changeparam") {
			string const p = cmd.getArg(1);
			string const v = cmd.getArg(2);
			if (p == "range") {
				flag.setEnabled(v == "none" || v == "start" || v == "end");
				flag.setOnOff(params_.range == insetindexpagerangetranslator().find(v));
			}
			if (p == "pagefmt") {
				flag.setEnabled(!v.empty());
				if (params_.pagefmt == "default" || params_.pagefmt == "textbf"
				    || params_.pagefmt == "textit" || params_.pagefmt == "emph")
					flag.setOnOff(params_.pagefmt == v);
				else
					flag.setOnOff(v == "custom");
			}
			return true;
		}
		return InsetCollapsible::getStatus(cur, cmd, flag);

	case LFUN_INSET_DIALOG_UPDATE: {
		Buffer const & realbuffer = *buffer().masterBuffer();
		flag.setEnabled(realbuffer.params().use_indices);
		return true;
	}

	case LFUN_INSET_INSERT_COPY:
		// This can only be invoked by ToC widget
		flag.setEnabled(cmd.origin() == FuncRequest::TOC
		                && cur.bv().cursor().inset().insetAllowed(lyxCode()));
		return true;

	case LFUN_PARAGRAPH_BREAK:
		return macrosPossible("subentry");

	case LFUN_INDEXMACRO_INSERT:
		return macrosPossible(cmd.getArg(0));

	case LFUN_INDEX_TAG_ALL:
		return true;

	default:
		return InsetCollapsible::getStatus(cur, cmd, flag);
	}
}


void InsetIndex::getSortkey(otexstream & os, OutputParams const & runparams) const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_SORTKEY_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			iim.getLatex(os, runparams);
			return;
		}
	}
}


docstring InsetIndex::getSortkeyAsText(OutputParams const & runparams) const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_SORTKEY_CODE) {
			otexstringstream os;
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			iim.getLatex(os, runparams);
			return os.str();
		}
	}
	return from_ascii("");
}


void InsetIndex::emptySubentriesWarning(docstring const & mainentry) const
{
	// Empty subentries crash makeindex. So warn and ignore this.
	TeXErrors terr;
	ErrorList & errorList = buffer().errorList("Export");
	docstring const s = bformat(_("There is an empty index subentry in the entry '%1$s'.\n"
				      "It will be ignored in the output."), mainentry);
	Paragraph const & par = buffer().paragraphs().front();
	errorList.push_back(ErrorItem(_("Empty index subentry!"), s,
				      {par.id(), 0}, {par.id(), -1}));
	buffer().bufferErrors(terr, errorList);
}


void InsetIndex::getSubentries(otexstream & os, OutputParams const & runparams,
			       docstring const & mainentry) const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	int i = 0;
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			if (iim.params().type == InsetIndexMacroParams::Subentry) {
				if (iim.hasNoContent()) {
					emptySubentriesWarning(mainentry);
					continue;
				}
				++i;
				if (i > 2)
					return;
				os << "!";
				iim.getLatex(os, runparams);
			}
		}
	}
}


std::vector<docstring> InsetIndex::getSubentriesAsText(OutputParams const & runparams,
						       bool const asLabel) const
{
	std::vector<docstring> subentries;

	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	int i = 0;
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			if (iim.params().type == InsetIndexMacroParams::Subentry) {
				++i;
				if (i > 2)
					break;
				if (asLabel) {
					docstring const l;
					docstring const sl = iim.getNewLabel(l);
					subentries.emplace_back(sl);
				} else {
					otexstringstream os;
					iim.getLatex(os, runparams);
					subentries.emplace_back(os.str());
				}
			}
		}
	}

	return subentries;
}


docstring InsetIndex::getMainSubentryAsText(OutputParams const & runparams) const
{
	otexstringstream os;
	InsetText::latex(os, runparams);
	return os.str();
}


void InsetIndex::getSeeRefs(otexstream & os, OutputParams const & runparams) const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			if (iim.params().type == InsetIndexMacroParams::See
			    || iim.params().type == InsetIndexMacroParams::Seealso) {
				iim.getLatex(os, runparams);
				return;
			}
		}
	}
}


docstring InsetIndex::getSeeAsText(OutputParams const & runparams,
				   bool const asLabel) const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			if (iim.params().type == InsetIndexMacroParams::See) {
				if (asLabel) {
					docstring const l;
					return iim.getNewLabel(l);
				} else {
					otexstringstream os;
					iim.getLatex(os, runparams);
					return os.str();
				}
			}
		}
	}
	return from_ascii("");
}


std::vector<docstring> InsetIndex::getSeeAlsoesAsText(OutputParams const & runparams,
						      bool const asLabel) const
{
	std::vector<docstring> seeAlsoes;

	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim =
				static_cast<InsetIndexMacro const &>(inset);
			if (iim.params().type == InsetIndexMacroParams::Seealso) {
				if (asLabel) {
					docstring const l;
					seeAlsoes.emplace_back(iim.getNewLabel(l));
				} else {
					otexstringstream os;
					iim.getLatex(os, runparams);
					seeAlsoes.emplace_back(os.str());
				}
			}
		}
	}

	return seeAlsoes;
}


namespace {

bool hasInsetWithCode(const InsetIndex * const inset_index, const InsetCode code,
					  const std::set<InsetIndexMacroParams::Type> types = {})
{
	Paragraph const & par = inset_index->paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == code) {
			if (types.empty())
				return true;

			LASSERT(code == INDEXMACRO_CODE, return false);
			InsetIndexMacro const & iim =
					static_cast<InsetIndexMacro const &>(inset);
			if (types.find(iim.params().type) != types.end())
				return true;
		}
	}
	return false;
}

} // namespace


bool InsetIndex::hasSubentries() const
{
	return hasInsetWithCode(this, INDEXMACRO_CODE, {InsetIndexMacroParams::Subentry});
}


bool InsetIndex::hasSeeRef() const
{
	return hasInsetWithCode(this, INDEXMACRO_CODE, {InsetIndexMacroParams::See, InsetIndexMacroParams::Seealso});
}


bool InsetIndex::hasSortKey() const
{
	return hasInsetWithCode(this, INDEXMACRO_SORTKEY_CODE);
}


bool InsetIndex::macrosPossible(string const type) const
{
	if (type != "see" && type != "seealso"
	    && type != "sortkey" && type != "subentry")
		return false;

	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	int subidxs = 0;
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (type == "sortkey" && inset.lyxCode() == INDEXMACRO_SORTKEY_CODE)
			return false;
		if (inset.lyxCode() == INDEXMACRO_CODE) {
			InsetIndexMacro const & iim = static_cast<InsetIndexMacro const &>(inset);
			if ((type == "see" || type == "seealso")
			     && (iim.params().type == InsetIndexMacroParams::See
				 || iim.params().type == InsetIndexMacroParams::Seealso))
				return false;
			if (type == "subentry"
			     && iim.params().type == InsetIndexMacroParams::Subentry) {
				++subidxs;
				if (subidxs > 1)
					return false;
			}
		}
	}
	return true;
}


ColorCode InsetIndex::labelColor() const
{
	if (params_.index.empty() || params_.index == from_ascii("idx"))
		return InsetCollapsible::labelColor();
	// FIXME UNICODE
	ColorCode c = lcolor.getFromLyXName(to_utf8(params_.index)
					    + "@" + buffer().fileName().absFileName());
	if (c == Color_none)
		c = InsetCollapsible::labelColor();
	return c;
}


docstring InsetIndex::toolTip(BufferView const &, int, int) const
{
	docstring tip = _("Index Entry");
	if (buffer().params().use_indices && !params_.index.empty()) {
		Buffer const & realbuffer = *buffer().masterBuffer();
		IndicesList const & indiceslist = realbuffer.params().indiceslist();
		tip += " (";
		Index const * index = indiceslist.findShortcut(params_.index);
		if (!index)
			tip += _("unknown type!");
		else
			tip += index->index();
		tip += ")";
	}
	tip += ": ";
	docstring res = toolTipText(tip);
	if (!insetindexpagerangetranslator_loc().find(params_.range).empty())
		res += "\n" + insetindexpagerangetranslator_loc().find(params_.range);
	if (!params_.pagefmt.empty() && params_.pagefmt != "default") {
		res += "\n" + _("Pagination format:") + " ";
		if (params_.pagefmt == "textbf")
			res += _("bold");
		else if (params_.pagefmt == "textit")
			res += _("italic");
		else if (params_.pagefmt == "emph")
			res += _("emphasized");
		else
			res += from_utf8(params_.pagefmt);
	}
	return res;
}


docstring const InsetIndex::buttonLabel(BufferView const & bv) const
{
	InsetLayout const & il = getLayout();
	docstring label = translateIfPossible(il.labelstring());

	if (buffer().params().use_indices && !params_.index.empty()) {
		Buffer const & realbuffer = *buffer().masterBuffer();
		IndicesList const & indiceslist = realbuffer.params().indiceslist();
		label += " (";
		Index const * index = indiceslist.findShortcut(params_.index);
		if (!index)
			label += _("unknown type!");
		else
			label += index->index();
		label += ")";
	}

	docstring res;
	if (!il.contentaslabel() || geometry(bv) != ButtonOnly)
		res = label;
	else {
		res = getNewLabel(label);
		OutputParams const rp(0);
		vector<docstring> sublbls = getSubentriesAsText(rp, true);
		for (auto const & sublbl : sublbls) {
			res += " " + docstring(1, char_type(0x2023));// TRIANGULAR BULLET
			res += " " + sublbl;
		}
		docstring see = getSeeAsText(rp, true);
		if (see.empty() && !getSeeAlsoesAsText(rp, true).empty())
			see = getSeeAlsoesAsText(rp, true).front();
		if (!see.empty()) {
			res += " " + docstring(1, char_type(0x261e));// WHITE RIGHT POINTING INDEX
			res += " " + see;
		}
	}
	if (!insetindexpagerangetranslator_latex().find(params_.range).empty())
		res += " " + from_ascii(insetindexpagerangetranslator_latex().find(params_.range));
	return res;
}


void InsetIndex::write(ostream & os) const
{
	os << to_utf8(layoutName());
	params_.write(os);
	InsetCollapsible::write(os);
}


void InsetIndex::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsible::read(lex);
}


string InsetIndex::params2string(InsetIndexParams const & params)
{
	ostringstream data;
	data << "index";
	params.write(data);
	return data.str();
}


void InsetIndex::string2params(string const & in, InsetIndexParams & params)
{
	params = InsetIndexParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetIndex::string2params");
	lex >> "index";
	params.read(lex);
}


void InsetIndex::addToToc(DocIterator const & cpit, bool output_active,
						  UpdateType utype, TocBackend & backend) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetIndex &>(*this)));
	docstring str;
	InsetLayout const & il = getLayout();
	docstring label = translateIfPossible(il.labelstring());
	if (!il.contentaslabel())
		str = label;
	else {
		str = getNewLabel(label);
		OutputParams const rp(0);
		vector<docstring> sublbls = getSubentriesAsText(rp, true);
		for (auto const & sublbl : sublbls) {
			str += " " + docstring(1, char_type(0x2023));// TRIANGULAR BULLET
			str += " " + sublbl;
		}
		docstring see = getSeeAsText(rp, true);
		if (see.empty() && !getSeeAlsoesAsText(rp, true).empty())
			see = getSeeAlsoesAsText(rp, true).front();
		if (!see.empty()) {
			str += " " + docstring(1, char_type(0x261e));// WHITE RIGHT POINTING INDEX
			str += " " + see;
		}
	}
	string type = "index";
	if (buffer().masterBuffer()->params().use_indices)
		type += ":" + to_utf8(params_.index);
	TocBuilder & b = backend.builder(type);
	b.pushItem(pit, str, output_active);
	// Proceed with the rest of the inset.
	InsetCollapsible::addToToc(cpit, output_active, utype, backend);
	b.pop();
}


void InsetIndex::validate(LaTeXFeatures & features) const
{
	if (buffer().masterBuffer()->params().use_indices
	    && !params_.index.empty()
	    && params_.index != "idx")
		features.require("splitidx");
	InsetCollapsible::validate(features);
}


string InsetIndex::contextMenuName() const
{
	return "context-index";
}


string InsetIndex::contextMenu(BufferView const & bv, int x, int y) const
{
	// We override the implementation of InsetCollapsible,
	// because we have eytra entries.
	string owncm = "context-edit-index;";
	return owncm + InsetCollapsible::contextMenu(bv, x, y);
}


bool InsetIndex::hasSettings() const
{
	return true;
}


bool InsetIndex::insetAllowed(InsetCode code) const
{
	switch (code) {
	case INDEXMACRO_CODE:
	case INDEXMACRO_SORTKEY_CODE:
		return true;
	case INDEX_CODE:
		return false;
	default:
		return InsetCollapsible::insetAllowed(code);
	}
}


/////////////////////////////////////////////////////////////////////
//
// InsetIndexParams
//
///////////////////////////////////////////////////////////////////////


void InsetIndexParams::write(ostream & os) const
{
	os << ' ';
	if (!index.empty())
		os << to_utf8(index);
	else
		os << "idx";
	os << '\n';
	os << "range "
	   << insetindexpagerangetranslator().find(range)
	   << '\n';
	os << "pageformat "
	   << pagefmt
	   << '\n';
}


void InsetIndexParams::read(Lexer & lex)
{
	if (lex.eatLine())
		index = lex.getDocString();
	else
		index = from_ascii("idx");
	if (lex.checkFor("range")) {
		string st = lex.getString();
		if (lex.eatLine()) {
			st = lex.getString();
			range = insetindexpagerangetranslator().find(lex.getString());
		}
	}
	if (lex.checkFor("pageformat") && lex.eatLine()) {
		pagefmt = lex.getString();
	}
}


/////////////////////////////////////////////////////////////////////
//
// InsetPrintIndex
//
///////////////////////////////////////////////////////////////////////

InsetPrintIndex::InsetPrintIndex(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetPrintIndex::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("type", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_ESCAPE);
		param_info_.add("name", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("literal", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


docstring InsetPrintIndex::screenLabel() const
{
	bool const printall = suffixIs(getCmdName(), '*');
	bool const multind = buffer().masterBuffer()->params().use_indices;
	if ((!multind
	     && getParam("type") == from_ascii("idx"))
	    || (getParam("type").empty() && !printall))
		return _("Index");
	Buffer const & realbuffer = *buffer().masterBuffer();
	IndicesList const & indiceslist = realbuffer.params().indiceslist();
	Index const * index = indiceslist.findShortcut(getParam("type"));
	if (!index && !printall)
		return _("Unknown index type!");
	docstring res = printall ? _("All indexes") : index->index();
	if (!multind)
		res += " (" + _("non-active") + ")";
	else if (contains(getCmdName(), "printsubindex"))
		res += " (" + _("subindex") + ")";
	return res;
}


bool InsetPrintIndex::isCompatibleCommand(string const & s)
{
	return s == "printindex" || s == "printsubindex"
		|| s == "printindex*" || s == "printsubindex*";
}


void InsetPrintIndex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		if (cmd.argument() == from_ascii("toggle-subindex")) {
			string scmd = getCmdName();
			if (contains(scmd, "printindex"))
				scmd = subst(scmd, "printindex", "printsubindex");
			else
				scmd = subst(scmd, "printsubindex", "printindex");
			cur.recordUndo();
			setCmdName(scmd);
			break;
		} else if (cmd.argument() == from_ascii("check-printindex*")) {
			string scmd = getCmdName();
			if (suffixIs(scmd, '*'))
				break;
			scmd += '*';
			cur.recordUndo();
			setParam("type", docstring());
			setCmdName(scmd);
			break;
		}
		InsetCommandParams p(INDEX_PRINT_CODE);
		// FIXME UNICODE
		InsetCommand::string2params(to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noScreenUpdate();
			break;
		}
		cur.recordUndo();
		setParams(p);
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetPrintIndex::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		if (cmd.argument() == from_ascii("toggle-subindex")) {
			status.setEnabled(buffer().masterBuffer()->params().use_indices);
			status.setOnOff(contains(getCmdName(), "printsubindex"));
			return true;
		} else if (cmd.argument() == from_ascii("check-printindex*")) {
			status.setEnabled(buffer().masterBuffer()->params().use_indices);
			status.setOnOff(suffixIs(getCmdName(), '*'));
			return true;
		} if (cmd.getArg(0) == "index_print"
		    && cmd.getArg(1) == "CommandInset") {
			InsetCommandParams p(INDEX_PRINT_CODE);
			InsetCommand::string2params(to_utf8(cmd.argument()), p);
			if (suffixIs(p.getCmdName(), '*')) {
				status.setEnabled(true);
				status.setOnOff(false);
				return true;
			}
			Buffer const & realbuffer = *buffer().masterBuffer();
			IndicesList const & indiceslist =
				realbuffer.params().indiceslist();
			Index const * index = indiceslist.findShortcut(p["type"]);
			status.setEnabled(index != 0);
			status.setOnOff(p["type"] == getParam("type"));
			return true;
		} else
			return InsetCommand::getStatus(cur, cmd, status);
	}

	case LFUN_INSET_DIALOG_UPDATE: {
		status.setEnabled(buffer().masterBuffer()->params().use_indices);
		return true;
	}

	default:
		return InsetCommand::getStatus(cur, cmd, status);
	}
}


void InsetPrintIndex::updateBuffer(ParIterator const &, UpdateType, bool const /*deleted*/)
{
	Index const * index =
		buffer().masterParams().indiceslist().findShortcut(getParam("type"));
	if (index)
		setParam("name", index->index());
}


void InsetPrintIndex::latex(otexstream & os, OutputParams const & runparams_in) const
{
	if (!buffer().masterBuffer()->params().use_indices) {
		if (getParam("type") == from_ascii("idx"))
			os << "\\printindex" << termcmd;
		return;
	}
	OutputParams runparams = runparams_in;
	os << getCommand(runparams);
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.require("makeidx");
	if (buffer().masterBuffer()->params().use_indices)
		features.require("splitidx");
	InsetCommand::validate(features);
}


string InsetPrintIndex::contextMenuName() const
{
	return buffer().masterBuffer()->params().use_indices ?
		"context-indexprint" : string();
}


bool InsetPrintIndex::hasSettings() const
{
	return buffer().masterBuffer()->params().use_indices;
}


class IndexEntry
{
public:
	/// Builds an entry for the index.
	IndexEntry(const InsetIndex * inset, OutputParams const * runparams) : inset_(inset), runparams_(runparams)
	{
		LASSERT(runparams, return);

		// Convert the inset as text. The resulting text usually only contains an XHTML anchor (<a id='...'/>) and text.
		odocstringstream entry;
		OutputParams ours = *runparams;
		ours.for_toc = false;
		inset_->plaintext(entry, ours);
		entry_ = entry.str();

		// Determine in which index this entry belongs to.
		if (inset_->buffer().masterBuffer()->params().use_indices) {
			index_ = inset_->params_.index;
		}

		// Attempt parsing the inset.
		if (isModern())
			parseAsModern();
		else
			parseAsLegacy();
	}

	/// When parsing this entry, some errors may be found; they are reported as a single string.
	// It is up to the caller to send this string to LYXERR and the output file, as needed.
	const docstring & output_error() const
	{
		return output_error_;
	}

	void output_error(XMLStream xs) const
	{
		LYXERR0(output_error());
		xs << XMLStream::ESCAPE_NONE << (from_utf8("<!-- Output Error: ") + output_error() + from_utf8(" -->\n"));
	}


private:
	bool isModern()
	{
#ifdef LYX_INSET_INDEX_DEBUG
		std::cout << to_utf8(entry_) << std::endl;
#endif // LYX_INSET_INDEX_DEBUG

		// If a modern parameter is present, this is definitely a modern index inset. Similarly, if it contains the
		// usual LaTeX symbols (!|@), then it is definitely a legacy index inset. Otherwise, if it has features of
		// neither, it is both: consider this is a modern inset, to trigger the least complex code. Mixing both types
		// is not allowed (i.e. behaviour is undefined).
		const bool is_definitely_modern = inset_->hasSortKey() || inset_->hasSeeRef() || inset_->hasSubentries()
		                            || inset_->params_.range != InsetIndexParams::PageRange::None;
		const bool is_definitely_legacy = entry_.find('@') != std::string::npos
				|| entry_.find('|') != std::string::npos || entry_.find('!') != std::string::npos;

		if (is_definitely_legacy && is_definitely_modern)
			output_error_ += from_utf8("Mix of index properties and raw LaTeX index commands is unsupported. ");

		// Truth table:
		// - is_definitely_modern == true:
		//   - is_definitely_legacy == true: error (return whatever)
		//   - is_definitely_legacy == false: return modern
		// - is_definitely_modern == false:
		//   - is_definitely_legacy == true: return legacy
		//   - is_definitely_legacy == false: return modern
		return !is_definitely_legacy;
	}

	void parseAsModern()
	{
		LASSERT(runparams_, return);

		if (inset_->hasSortKey()) {
			sort_as_ = inset_->getSortkeyAsText(*runparams_);
		}

		terms_ = inset_->getSubentriesAsText(*runparams_);
		// The main term is not present in the vector, as it's not a subentry. The main index term is inserted raw in
		// the index inset. Considering that the user either uses the new or the legacy mechanism, the main term is the
		// full string within this inset (i.e. without the subinsets).
		terms_.insert(terms_.begin(), inset_->getMainSubentryAsText(*runparams_));

		has_start_range_ = inset_->params_.range == InsetIndexParams::PageRange::Start;
		has_end_range_ = inset_->params_.range == InsetIndexParams::PageRange::End;

		see_ = inset_->getSeeAsText(*runparams_);
		see_alsoes_ = inset_->getSeeAlsoesAsText(*runparams_);
	}

	void parseAsLegacy() {
		// Determine if some features are known not to be supported. For now, this is only formatting like
		// \index{alpha@\textbf{alpha}} or \index{alpha@$\alpha$}.
		// @ is supported, but only for sorting, without specific formatting.
		if (entry_.find(from_utf8("@\\")) != lyx::docstring::npos) {
			output_error_ += from_utf8("Unsupported feature: an index entry contains an @\\. "
			                           "Complete entry: \"") + entry_ + from_utf8("\". ");
		}
		if (entry_.find(from_utf8("@$")) != lyx::docstring::npos) {
			output_error_ += from_utf8("Unsupported feature: an index entry contains an @$. "
			                           "Complete entry: \"") + entry_ + from_utf8("\". ");
		}

		// Split the string into its main constituents: terms, and command (see, see also, range).
		size_t positionVerticalBar = entry_.find(from_ascii("|")); // What comes before | is (sub)(sub)entries.
		docstring indexTerms = entry_.substr(0, positionVerticalBar);
		docstring command;
		if (positionVerticalBar != lyx::docstring::npos) {
			command = entry_.substr(positionVerticalBar + 1);
		}

		// Handle sorting issues, with @.
		vector<docstring> sortingElements = getVectorFromString(indexTerms, from_ascii("@"), false);
		if (sortingElements.size() == 2) {
			sort_as_ = sortingElements[0];
			indexTerms = sortingElements[1];
		}

		// Handle entries, subentries, and subsubentries.
		terms_ = getVectorFromString(indexTerms, from_ascii("!"), false);

		// Handle ranges. Happily, (| and |) can only be at the end of the string!
		has_start_range_ = entry_.find(from_ascii("|(")) != lyx::docstring::npos;
		has_end_range_ = entry_.find(from_ascii("|)")) != lyx::docstring::npos;

		// - Remove the ranges from the command if they do not appear at the beginning.
		size_t range_index = 0;
		while ((range_index = command.find(from_utf8("|("), range_index)) != std::string::npos)
			command.erase(range_index, 1);
		range_index = 0;
		while ((range_index = command.find(from_utf8("|)"), range_index)) != std::string::npos)
			command.erase(range_index, 1);

		// - Remove the ranges when they are the only vertical bar in the complete string.
		if (command[0] == '(' || command[0] == ')')
			command.erase(0, 1);

		// Handle see and seealso. As "see" is a prefix of "seealso", the order of the comparisons is important.
		// Both commands are mutually exclusive!
		if (command.substr(0, 3) == "see") {
			// Unescape brackets.
			size_t index_argument_begin = 0;
			while ((index_argument_begin = command.find(from_utf8("\\{"), index_argument_begin)) != std::string::npos)
				command.erase(index_argument_begin, 1);
			size_t index_argument_end = 0;
			while ((index_argument_end = command.find(from_utf8("\\}"), index_argument_end)) != std::string::npos)
				command.erase(index_argument_end, 1);

			// Retrieve the part between brackets, and remove the complete seealso.
			size_t position_opening_bracket = command.find(from_ascii("{"));
			size_t position_closing_bracket = command.find(from_ascii("}"));
			docstring argument = command.substr(position_opening_bracket + 1,
												position_closing_bracket - position_opening_bracket - 1);

			// Parse the argument of referenced entries (or a single one for see).
			if (command.substr(0, 7) == "seealso") {
				see_alsoes_ = getVectorFromString(argument, from_ascii(","), false);
			} else {
				see_ = argument;

				if (see_.find(from_ascii(",")) != std::string::npos) {
					output_error_ += from_utf8("Several index_argument_end terms found as \"see\"! Only one is "
					                           "acceptable. Complete entry: \"") + entry_ + from_utf8("\". ");
				}
			}

			// Remove the complete see/seealso from the commands, in case there is something else to parse.
			command = command.substr(position_closing_bracket + 1);
		}

		// Some parts of the strings are not parsed, as they do not have anything matching in DocBook or XHTML:
		// things like formatting the entry or the page number, other strings for sorting.
		// https://wiki.lyx.org/Tips/Indexing
		// If there are such things in the index entry, then this code may miserably fail. For example, for
		// "Peter|(textbf", no range will be detected.
		if (!command.empty()) {
			output_error_ += from_utf8("Unsupported feature: an index entry contains a | with an unsupported command, ")
			                 + command + from_utf8(". Complete entry: \"") + entry_ + from_utf8("\". ");
		}
	}

public:
	int level() const {
		return terms_.size();
	}

	const std::vector<docstring>& terms() const {
		return terms_;
	}

	std::vector<docstring>& terms() {
		return terms_;
	}

	const InsetIndex* inset() const {
		return inset_;
	}

private:
	// Input inset. These should only be used when parsing the inset (either parseAsModern or parseAsLegacy, called in
	// the constructor).
	const InsetIndex * inset_;
	OutputParams const * runparams_;
	docstring entry_;
	docstring index_; // Useful when there are multiple indices in the same document.

	// Errors, concatenated as a single string, available as soon as parsing is done, const afterwards (i.e. once
	// constructor is done).
	docstring output_error_;

	// Parsed index entry.
	std::vector<docstring> terms_; // Up to three entries, in general.
	docstring sort_as_;
	docstring command_;
	bool has_start_range_;
	bool has_end_range_;
	docstring see_;
	vector<docstring> see_alsoes_;

	// Operators used for sorting entries (alphabetical order).
	friend bool operator<(IndexEntry const & lhs, IndexEntry const & rhs);
};

bool operator<(IndexEntry const & lhs, IndexEntry const & rhs)
{
	if (lhs.terms_.empty())
		return false;

	for (unsigned i = 0; i < min(rhs.terms_.size(), lhs.terms_.size()); ++i) {
		int comp = compare_no_case(lhs.terms_[i], rhs.terms_[i]);
		if (comp != 0)
			return comp < 0;
	}
	return false;
}


namespace {
std::string generateCssClassAtDepth(unsigned depth) {
	std::string css_class = "entry";

	while (depth > 0) {
		depth -= 1;
		css_class.insert(0, "sub");
	}

	return css_class;
}

struct IndexNode {
	std::vector<IndexEntry> entries;
	std::vector<IndexNode*> children;
};

docstring termAtLevel(const IndexNode* node, unsigned depth)
{
	// The typical entry has a depth of 1 to 3: the call stack would then be at most 4 (due to the root node). This
	// function could be made constant time by copying the term in each node, but that would make data duplication that
	// may fall out of sync; the performance benefit would probably be negligible.
	if (!node->entries.empty()) {
		LASSERT(node->entries.begin()->terms().size() >= depth + 1, return from_ascii(""));
		return node->entries.begin()->terms()[depth];
	}

	if (!node->children.empty()) {
		return termAtLevel(*node->children.begin(), depth);
	}

	LASSERT(false, return from_ascii(""));
}

void insertIntoNode(const IndexEntry& entry, IndexNode* node, unsigned depth = 0)
{
	// depth == 0 is for the root, not yet the index, hence the increase when going to vector size.
	for (IndexNode* child : node->children) {
		if (entry.terms()[depth] == termAtLevel(child, depth)) {
			if (depth + 1 == entry.terms().size()) { // == child.entries.begin()->terms().size()
				// All term entries match: it's an entry.
				child->entries.emplace_back(entry);
				return;
			} else {
				insertIntoNode(entry, child, depth + 1);
				return;
			}
		}
	}

	// Out of the loop: no matching child found, create a new (possibly nested) child for this entry. Due to the
	// possibility of nestedness, only insert the current entry when the right level is reached. This is needed if the
	// first entry for a word has several levels that never appeared.
	// In particular, this case is called for the first entry.
	IndexNode* new_node = node;
	do {
		new_node->children.emplace_back(new IndexNode{{}, {}});
		new_node = new_node->children.back();
		depth += 1;
	} while (depth + 1 <= entry.terms().size()); // depth == 0: root node, no text associated.
	new_node->entries.emplace_back(entry);
}

IndexNode* buildIndexTree(vector<IndexEntry>& entries)
{
	// Sort the entries, first on the main entry, then the subentry, then the subsubentry,
	// thanks to the implementation of operator<.
	// If this operation is not performed, the algorithm below is no more correct (and ensuring that it works with
	// unsorted entries would make its complexity blow up).
	stable_sort(entries.begin(), entries.end());

	// Cook the index into a nice tree data structure: entries at a given level in the index as a node, with subentries
	// as children.
	auto* index_root = new IndexNode{{}, {}};
	for (const IndexEntry& entry : entries) {
		insertIntoNode(entry, index_root);
	}

	return index_root;
}

void outputIndexPage(XMLStream & xs, const IndexNode* root_node, unsigned depth = 0) // NOLINT(misc-no-recursion)
{
	LASSERT(root_node->entries.size() + root_node->children.size() > 0, return);

	xs << xml::StartTag("li", "class='" + generateCssClassAtDepth(depth) + "'");
	xs << xml::CR();
	xs << XMLStream::ESCAPE_NONE << termAtLevel(root_node, depth);
	// By tree assumption, all the entries at this node have the same set of terms.

	if (!root_node->entries.empty()) {
		xs << XMLStream::ESCAPE_NONE << " &#8212; "; // Em dash, i.e. long (---).
		unsigned entry_number = 1;

		auto writeLinkToEntry = [&xs](const IndexEntry &entry, unsigned entry_number) {
			std::string const link_attr = "href='#" + entry.inset()->paragraphs()[0].magicLabel() + "'";
			xs << xml::StartTag("a", link_attr);
			xs << from_ascii(std::to_string(entry_number));
			xs << xml::EndTag("a");
		};

		for (unsigned i = 0; i < root_node->entries.size(); ++i) {
			const IndexEntry &entry = root_node->entries[i];

			switch (entry.inset()->params().range) {
				case InsetIndexParams::PageRange::None:
					writeLinkToEntry(entry, entry_number);
					break;
				case InsetIndexParams::PageRange::Start: {
					// Try to find the end of the range, if it is just after. Otherwise, the output will be slightly
					// scrambled, but understandable. Doing better would mean implementing more of the indexing logic here
					// and more complex indexing here (skipping the end is not just incrementing i). Worst case output:
					//     1--, 2, --3
					const bool nextEntryIsEnd = i + 1 < root_node->entries.size() &&
					                            root_node->entries[i + 1].inset()->params().range ==
					                            InsetIndexParams::PageRange::End;
					// No need to check if both entries are for the same terms: they are in the same IndexNode.

					writeLinkToEntry(entry, entry_number);
					xs << XMLStream::ESCAPE_NONE << " &#8211; "; // En dash, i.e. semi-long (--).

					if (nextEntryIsEnd) {
						// Skip the next entry in the loop, write it right now, after the dash.
						entry_number += 1;
						i += 1;
						writeLinkToEntry(root_node->entries[i], entry_number);
					}
				}
					break;
				case InsetIndexParams::PageRange::End:
					// This range end was not caught by the range start, do it now to avoid losing content.
					xs << XMLStream::ESCAPE_NONE << " &#8211; "; // En dash, i.e. semi-long (--).
					writeLinkToEntry(root_node->entries[i], entry_number);
			}

			if (i < root_node->entries.size() - 1) {
				xs << ", ";
			}
			entry_number += 1;
		}
	}

	if (!root_node->entries.empty() && !root_node->children.empty()) {
		xs << xml::CR();
	}

	if (!root_node->children.empty()) {
		xs << xml::StartTag("ul", "class='" + generateCssClassAtDepth(depth) + "'");
		xs << xml::CR();

		for (const IndexNode* child : root_node->children) {
			outputIndexPage(xs, child, depth + 1);
		}

		xs << xml::EndTag("ul");
		xs << xml::CR();
	}

	xs << xml::EndTag("li");
	xs << xml::CR();
}

#ifdef LYX_INSET_INDEX_DEBUG
void printTree(const IndexNode* root_node, unsigned depth = 0)
{
	static const std::string pattern = "    ";
	std::string prefix;
	for (unsigned i = 0; i < depth; ++i) {
		prefix += pattern;
	}
	const std::string prefix_long = prefix + pattern + pattern;

	docstring term_at_level;
	if (depth == 0) {
		// The root has no term.
		std::cout << "<ROOT>" << std::endl;
	} else {
		LASSERT(depth - 1 <= 10, return); // Check for overflows.
		term_at_level = termAtLevel(root_node, depth - 1);
		std::cout << prefix << to_utf8(term_at_level) << " (x " << std::to_string(root_node->entries.size()) << ")"
		          << std::endl;
	}

	for (const IndexEntry& entry : root_node->entries) {
		if (entry.terms().size() != depth) {
			std::cout << prefix_long << "ERROR: an entry doesn't have the same number of terms" << std::endl;
		}
		if (depth > 0 && entry.terms()[depth - 1] != term_at_level) {
			std::cout << prefix_long << "ERROR: an entry doesn't have the right term at depth " << std::to_string(depth)
				<< std::endl;
		}
	}

	for (const IndexNode* node : root_node->children) {
		printTree(node, depth + 1);
	}
}
#endif // LYX_INSET_INDEX_DEBUG
}


docstring InsetPrintIndex::xhtml(XMLStream &, OutputParams const & op) const
{
	BufferParams const & bp = buffer().masterBuffer()->params();

	shared_ptr<Toc const> toc = buffer().tocBackend().toc("index");
	if (toc->empty())
		return docstring();

	// Collect the index entries in a form we can use them.
	vector<IndexEntry> entries;
	const docstring & indexType = params().getParamOr("type", from_ascii("idx"));
	for (const TocItem& item : *toc) {
		const auto* inset = static_cast<const InsetIndex*>(&(item.dit().inset()));
		if (item.isOutput() && inset->params().index == indexType)
			entries.emplace_back(IndexEntry{inset, &op});
	}

	// If all the index entries are in notes or not displayed, get out sooner.
	if (entries.empty())
		return docstring();

	const IndexNode* index_root = buildIndexTree(entries);
#ifdef LYX_INSET_INDEX_DEBUG
	printTree(index_root);
#endif

	// Start generating the XHTML index.
	Layout const & lay = bp.documentClass().htmlTOCLayout();
	string const & tocclass = lay.defaultCSSClass();
	string const tocattr = "class='index " + tocclass + "'";
	docstring const indexName = params().getParamOr("name", from_ascii("Index"));

	// we'll use our own stream, because we are going to defer everything.
	// that's how we deal with the fact that we're probably inside a standard
	// paragraph, and we don't want to be.
	odocstringstream ods;
	XMLStream xs(ods);

	xs << xml::StartTag("div", tocattr);
	xs << xml::CR();
	xs << xml::StartTag(lay.htmltag(), lay.htmlattr());
	xs << translateIfPossible(indexName, op.local_font->language()->lang());
	xs << xml::EndTag(lay.htmltag());
	xs << xml::CR();
	xs << xml::StartTag("ul", "class='main'");
	xs << xml::CR();

	LASSERT(index_root->entries.empty(), return docstring()); // No index entry should have zero terms.
	for (const IndexNode* node : index_root->children) {
		outputIndexPage(xs, node);
	}

	xs << xml::EndTag("ul");
	xs << xml::CR();
	xs << xml::EndTag("div");

	return ods.str();
}

} // namespace lyx
