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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "ColorSet.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "IndicesList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "output_latex.h"
#include "output_xhtml.h"
#include "sgml.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

#include <algorithm>
#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetIndex
//
///////////////////////////////////////////////////////////////////////


InsetIndex::InsetIndex(Buffer * buf, InsetIndexParams const & params)
	: InsetCollapsable(buf), params_(params)
{}


void InsetIndex::latex(otexstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	runparams.inIndexEntry = true;

	if (buffer().masterBuffer()->params().use_indices && !params_.index.empty()
	    && params_.index != "idx") {
		os << "\\sindex[";
		os << escape(params_.index);
		os << "]{";
	} else {
		os << "\\index";
		os << '{';
	}

	// get contents of InsetText as LaTeX and plaintext
	TexRow texrow;
	odocstringstream ourlatex;
	otexstream ots(ourlatex, texrow);
	InsetText::latex(ots, runparams);
	odocstringstream ourplain;
	InsetText::plaintext(ourplain, runparams);
	docstring latexstr = ourlatex.str();
	docstring plainstr = ourplain.str();

	// this will get what follows | if anything does
	docstring cmd;

	// check for the | separator
	// FIXME This would go wrong on an escaped "|", but
	// how far do we want to go here?
	size_t pos = latexstr.find(from_ascii("|"));
	if (pos != docstring::npos) {
		// put the bit after "|" into cmd...
		cmd = latexstr.substr(pos + 1);
		// ...and erase that stuff from latexstr
		latexstr = latexstr.erase(pos);
		// ...and similarly from plainstr
		size_t ppos = plainstr.find(from_ascii("|"));
		if (ppos < plainstr.size())
			plainstr.erase(ppos);
		else
			LYXERR0("The `|' separator was not found in the plaintext version!");
	}

	// Separate the entires and subentries, i.e., split on "!"
	// FIXME This would do the wrong thing with escaped ! characters
	std::vector<docstring> const levels =
		getVectorFromString(latexstr, from_ascii("!"), true);
	std::vector<docstring> const levels_plain =
		getVectorFromString(plainstr, from_ascii("!"), true);

	vector<docstring>::const_iterator it = levels.begin();
	vector<docstring>::const_iterator end = levels.end();
	vector<docstring>::const_iterator it2 = levels_plain.begin();
	bool first = true;
	for (; it != end; ++it) {
		// write the separator except the first time
		if (!first)
			os << '!';
		else
			first = false;

		// correctly sort macros and formatted strings
		// if we do find a command, prepend a plain text
		// version of the content to get sorting right,
		// e.g. \index{LyX@\LyX}, \index{text@\textbf{text}}
		// Don't do that if the user entered '@' himself, though.
		if (contains(*it, '\\') && !contains(*it, '@')) {
			// Plaintext might return nothing (e.g. for ERTs)
			docstring const spart = 
				(it2 < levels_plain.end() && !(*it2).empty())
				? *it2 : *it;
			// Now we need to validate that all characters in
			// the sorting part are representable in the current
			// encoding. If not try the LaTeX macro which might
			// or might not be a good choice, and issue a warning.
			pair<docstring, docstring> spart_latexed =
				runparams.encoding->latexString(spart, runparams.dryrun);
			if (!spart_latexed.second.empty())
					LYXERR0("Uncodable character in index entry. Sorting might be wrong!");
			if (spart != spart_latexed.first && !runparams.dryrun) {
				// FIXME: warning should be passed to the error dialog
				frontend::Alert::warning(_("Index sorting failed"),
				bformat(_("LyX's automatic index sorting algorithm faced\n"
				  "problems with the entry '%1$s'.\n"
				  "Please specify the sorting of this entry manually, as\n"
				  "explained in the User Guide."), spart));
			}
			// remove remaining \'s for the sorting part
			docstring const ppart =
				subst(spart_latexed.first, from_ascii("\\"), docstring());
			os << ppart;
			os << '@';
		}
		docstring const tpart = *it;
		os << tpart;
		if (it2 < levels_plain.end())
			++it2;
	}
	// write the bit that followed "|"
	if (!cmd.empty()) {
		os << "|" << cmd;
	}
	os << '}';
}


int InsetIndex::docbook(odocstream & os, OutputParams const & runparams) const
{
	os << "<indexterm><primary>";
	int const i = InsetText::docbook(os, runparams);
	os << "</primary></indexterm>";
	return i;
}


docstring InsetIndex::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	// we just print an anchor, taking the paragraph ID from 
	// our own interior paragraph, which doesn't get printed
	std::string const magic = paragraphs().front().magicLabel();
	std::string const attr = "id='" + magic + "'";
	xs << html::CompTag("a", attr);
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
			cur.recordUndoInset(ATOMIC_UNDO, this);
			params_.index = from_utf8(cmd.getArg(1));
			break;
		}
		InsetIndexParams params;
		InsetIndex::string2params(to_utf8(cmd.argument()), params);
		cur.recordUndoInset(ATOMIC_UNDO, this);
		params_.index = params.index;
		// what we really want here is a TOC update, but that means
		// a full buffer update
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("index", params2string(params_));
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
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
		return InsetCollapsable::getStatus(cur, cmd, flag);

	case LFUN_INSET_DIALOG_UPDATE: {
		Buffer const & realbuffer = *buffer().masterBuffer();
		flag.setEnabled(realbuffer.params().use_indices);
		return true;
	}

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}


ColorCode InsetIndex::labelColor() const
{
	if (params_.index.empty() || params_.index == from_ascii("idx"))
		return InsetCollapsable::labelColor();
	// FIXME UNICODE
	ColorCode c = lcolor.getFromLyXName(to_utf8(params_.index));
	if (c == Color_none)
		c = InsetCollapsable::labelColor();
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
	return toolTipText(tip);
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

	if (!il.contentaslabel() || geometry(bv) != ButtonOnly)
		return label;
	return getNewLabel(label);
}


void InsetIndex::write(ostream & os) const
{
	os << to_utf8(layoutName());
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetIndex::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
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


void InsetIndex::addToToc(DocIterator const & cpit, bool output_active) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetIndex &>(*this)));
	docstring str;
	string type = "index";
	if (buffer().masterBuffer()->params().use_indices)
		type += ":" + to_utf8(params_.index);
	text().forToc(str, 0);
	buffer().tocBackend().toc(type).push_back(TocItem(pit, 0, str, output_active));
	// Proceed with the rest of the inset.
	InsetCollapsable::addToToc(cpit, output_active);
}


void InsetIndex::validate(LaTeXFeatures & features) const
{
	if (buffer().masterBuffer()->params().use_indices
	    && !params_.index.empty()
	    && params_.index != "idx")
		features.require("splitidx");
	InsetCollapsable::validate(features);
}


string InsetIndex::contextMenuName() const
{
	return "context-index";
}


bool InsetIndex::hasSettings() const
{
	return buffer().masterBuffer()->params().use_indices;
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
}


void InsetIndexParams::read(Lexer & lex)
{
	if (lex.eatLine())
		index = lex.getDocString();
	else
		index = from_ascii("idx");
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
		param_info_.add("name", ParamInfo::LATEX_REQUIRED);
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
			string cmd = getCmdName();
			if (contains(cmd, "printindex"))
				cmd = subst(cmd, "printindex", "printsubindex");
			else
				cmd = subst(cmd, "printsubindex", "printindex");
			cur.recordUndo();
			setCmdName(cmd);
			break;
		} else if (cmd.argument() == from_ascii("check-printindex*")) {
			string cmd = getCmdName();
			if (suffixIs(cmd, '*'))
				break;
			cmd += '*';
			cur.recordUndo();
			setParam("type", docstring());
			setCmdName(cmd);
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


void InsetPrintIndex::latex(otexstream & os, OutputParams const & runparams_in) const
{
	if (!buffer().masterBuffer()->params().use_indices) {
		if (getParam("type") == from_ascii("idx"))
			os << "\\printindex{}";
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


namespace {

void parseItem(docstring & s, bool for_output)
{
	// this does not yet check for escaped things
	size_type loc = s.find(from_ascii("@"));
	if (loc != string::npos) {
		if (for_output)
			s.erase(0, loc + 1);
		else
			s.erase(loc);
	}
	loc = s.find(from_ascii("|"));
	if (loc != string::npos)
		s.erase(loc);
}

	
void extractSubentries(docstring const & entry, docstring & main,
		docstring & sub1, docstring & sub2)
{
	if (entry.empty())
		return;
	size_type const loc = entry.find(from_ascii(" ! "));
	if (loc == string::npos)
		main = entry;
	else {
		main = trim(entry.substr(0, loc));
		size_t const locend = loc + 3;
		size_type const loc2 = entry.find(from_ascii(" ! "), locend);
		if (loc2 == string::npos) {
			sub1 = trim(entry.substr(locend));
		} else {
			sub1 = trim(entry.substr(locend, loc2 - locend));
			sub2 = trim(entry.substr(loc2 + 3));
		}
	}
}


struct IndexEntry
{
	IndexEntry() 
	{}
	
	IndexEntry(docstring const & s, DocIterator const & d) 
			: dit(d)
	{
		extractSubentries(s, main, sub, subsub);
		parseItem(main, false);
		parseItem(sub, false);
		parseItem(subsub, false);
	}
	
	bool equal(IndexEntry const & rhs) const
	{
		return main == rhs.main && sub == rhs.sub && subsub == rhs.subsub;
	}
	
	bool same_sub(IndexEntry const & rhs) const
	{
		return main == rhs.main && sub == rhs.sub;
	}
	
	bool same_main(IndexEntry const & rhs) const
	{
		return main == rhs.main;
	}
	
	docstring main;
	docstring sub;
	docstring subsub;
	DocIterator dit;
};

bool operator<(IndexEntry const & lhs, IndexEntry const & rhs)
{
	int comp = compare_no_case(lhs.main, rhs.main);
	if (comp == 0)
		comp = compare_no_case(lhs.sub, rhs.sub);
	if (comp == 0)
		comp = compare_no_case(lhs.subsub, rhs.subsub);
	return (comp < 0);
}

} // anon namespace


docstring InsetPrintIndex::xhtml(XHTMLStream &, OutputParams const & op) const
{
	BufferParams const & bp = buffer().masterBuffer()->params();

	// we do not presently support multiple indices, so we refuse to print
	// anything but the main index, so as not to generate multiple indices.
	// NOTE Multiple index support would require some work. The reason
	// is that the TOC does not know about multiple indices. Either it would
	// need to be told about them (not a bad idea), or else the index entries
	// would need to be collected differently, say, during validation.
	if (bp.use_indices && getParam("type") != from_ascii("idx"))
		return docstring();
	
	Toc const & toc = buffer().tocBackend().toc("index");
	if (toc.empty())
		return docstring();

	// Collect the index entries in a form we can use them.
	Toc::const_iterator it = toc.begin();
	Toc::const_iterator const en = toc.end();
	vector<IndexEntry> entries;
	for (; it != en; ++it)
		if (it->isOutput())
			entries.push_back(IndexEntry(it->str(), it->dit()));

	if (entries.empty())
		// not very likely that all the index entries are in notes or
		// whatever, but....
		return docstring();

	stable_sort(entries.begin(), entries.end());

	Layout const & lay = bp.documentClass().htmlTOCLayout();
	string const & tocclass = lay.defaultCSSClass();
	string const tocattr = "class='index " + tocclass + "'";

	// we'll use our own stream, because we are going to defer everything.
	// that's how we deal with the fact that we're probably inside a standard
	// paragraph, and we don't want to be.
	odocstringstream ods;
	XHTMLStream xs(ods);

	xs << html::StartTag("div", tocattr);
	xs << html::StartTag(lay.htmltag(), lay.htmlattr()) 
		 << translateIfPossible(from_ascii("Index"),
	                          op.local_font->language()->lang())
		 << html::EndTag(lay.htmltag());
	xs << html::StartTag("ul", "class='main'");
	Font const dummy;

	vector<IndexEntry>::const_iterator eit = entries.begin();
	vector<IndexEntry>::const_iterator const een = entries.end();
	// tracks whether we are already inside a main entry (1),
	// a sub-entry (2), or a sub-sub-entry (3). see below for the
	// details.
	int level = 1;
	// the last one we saw
	IndexEntry last;
	int entry_number = -1;
	for (; eit != een; ++eit) {
		Paragraph const & par = eit->dit.innerParagraph();
		if (entry_number == -1 || !eit->equal(last)) {
			if (entry_number != -1) {
				// not the first time through the loop, so
				// close last entry or entries, depending.
				if (level == 3) {
					// close this sub-sub-entry
					xs << html::EndTag("li") << html::CR();
					// is this another sub-sub-entry within the same sub-entry?
					if (!eit->same_sub(last)) {
						// close this level
						xs << html::EndTag("ul") << html::CR();
						level = 2;
					}
				}
				// the point of the second test here is that we might get
				// here two ways: (i) by falling through from above; (ii) because,
				// though the sub-entry hasn't changed, the sub-sub-entry has,
				// which means that it is the first sub-sub-entry within this
				// sub-entry. In that case, we do not want to close anything.
				if (level == 2 && !eit->same_sub(last)) {
					// close sub-entry 
					xs << html::EndTag("li") << html::CR();
					// is this another sub-entry with the same main entry?
					if (!eit->same_main(last)) {
						// close this level
						xs << html::EndTag("ul") << html::CR();
						level = 1;
					}
				}
				// again, we can get here two ways: from above, or because we have
				// found the first sub-entry. in the latter case, we do not want to
				// close the entry.
				if (level == 1 && !eit->same_main(last)) {
					// close entry
					xs << html::EndTag("li") << html::CR();
				}
			}

			// we'll be starting new entries
			entry_number = 0;

			// We need to use our own stream, since we will have to
			// modify what we get back.
			odocstringstream ent;
			XHTMLStream entstream(ent);
			OutputParams ours = op;
			ours.for_toc = true;
			par.simpleLyXHTMLOnePar(buffer(), entstream, ours, dummy);
	
			// these will contain XHTML versions of the main entry, etc
			// remember that everything will already have been escaped,
			// so we'll need to use NextRaw() during output.
			docstring main;
			docstring sub;
			docstring subsub;
			extractSubentries(ent.str(), main, sub, subsub);
			parseItem(main, true);
			parseItem(sub, true);
			parseItem(subsub, true);
	
			if (level == 3) {
				// another subsubentry
				xs << html::StartTag("li", "class='subsubentry'") 
				   << XHTMLStream::ESCAPE_NONE << subsub;
			} else if (level == 2) {
				// there are two ways we can be here: 
				// (i) we can actually be inside a sub-entry already and be about
				//     to output the first sub-sub-entry. in this case, our sub
				//     and the last sub will be the same.
				// (ii) we can just have closed a sub-entry, possibly after also
				//     closing a list of sub-sub-entries. here our sub and the last
				//     sub are different.
				// only in the latter case do we need to output the new sub-entry.
				// note that in this case, too, though, the sub-entry might already
				// have a sub-sub-entry.
				if (eit->sub != last.sub)
					xs << html::StartTag("li", "class='subentry'") 
					   << XHTMLStream::ESCAPE_NONE << sub;
				if (!subsub.empty()) {
					// it's actually a subsubentry, so we need to start that list
					xs << html::CR()
					   << html::StartTag("ul", "class='subsubentry'") 
					   << html::StartTag("li", "class='subsubentry'") 
					   << XHTMLStream::ESCAPE_NONE << subsub;
					level = 3;
				} 
			} else {
				// there are also two ways we can be here: 
				// (i) we can actually be inside an entry already and be about
				//     to output the first sub-entry. in this case, our main
				//     and the last main will be the same.
				// (ii) we can just have closed an entry, possibly after also
				//     closing a list of sub-entries. here our main and the last
				//     main are different.
				// only in the latter case do we need to output the new main entry.
				// note that in this case, too, though, the main entry might already
				// have a sub-entry, or even a sub-sub-entry.
				if (eit->main != last.main)
					xs << html::StartTag("li", "class='main'") << main;
				if (!sub.empty()) {
					// there's a sub-entry, too
					xs << html::CR()
					   << html::StartTag("ul", "class='subentry'") 
					   << html::StartTag("li", "class='subentry'") 
					   << XHTMLStream::ESCAPE_NONE << sub;
					level = 2;
					if (!subsub.empty()) {
						// and a sub-sub-entry
						xs << html::CR()
						   << html::StartTag("ul", "class='subsubentry'") 
						   << html::StartTag("li", "class='subsubentry'") 
						   << XHTMLStream::ESCAPE_NONE << subsub;
						level = 3;
					}
				} 
			}
		}
		// finally, then, we can output the index link itself
		string const parattr = "href='#" + par.magicLabel() + "'";
		xs << (entry_number == 0 ? ":" : ",");
		xs << " " << html::StartTag("a", parattr)
		   << ++entry_number << html::EndTag("a");
		last = *eit;
	}
	// now we have to close all the open levels
	while (level > 0) {
		xs << html::EndTag("li") << html::EndTag("ul") << html::CR();
		--level;
	}
	xs << html::EndTag("div") << html::CR();
	return ods.str();
}

} // namespace lyx
