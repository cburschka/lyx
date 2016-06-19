/**
 * \file InsetNomencl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author O. U. Baran
 * \author Uwe Stöhr
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetNomencl.h"
#include "InsetNote.h"

#include "Buffer.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Font.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetIterator.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Length.h"
#include "LyX.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "sgml.h"
#include "texstream.h"
#include "TocBackend.h"

#include "frontends/FontMetrics.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


/////////////////////////////////////////////////////////////////////
//
// InsetNomencl
//
/////////////////////////////////////////////////////////////////////

InsetNomencl::InsetNomencl(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p),
	  nomenclature_entry_id(sgml::uniqueID(from_ascii("nomen")))
{}


ParamInfo const & InsetNomencl::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("prefix", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("symbol", ParamInfo::LATEX_REQUIRED,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("description", ParamInfo::LATEX_REQUIRED,
				ParamInfo::HANDLING_LATEXIFY);
	}
	return param_info_;
}


docstring InsetNomencl::screenLabel() const
{
	size_t const maxLabelChars = 25;
	docstring label = _("Nom: ") + getParam("symbol");
	support::truncateWithEllipsis(label, maxLabelChars);
	return label;
}


docstring InsetNomencl::toolTip(BufferView const & /*bv*/, int /*x*/, int /*y*/) const
{
	docstring tip = _("Nomenclature Symbol: ") + getParam("symbol") + "\n";
	tip += _("Description: ") + "\t"
		+ subst(getParam("description"), from_ascii("\\\\"), from_ascii("\n\t"));
	if (!getParam("prefix").empty())
		tip += "\n" + _("Sorting: ") + getParam("prefix");
	return tip;
}



int InsetNomencl::docbook(odocstream & os, OutputParams const &) const
{
	os << "<glossterm linkend=\"" << nomenclature_entry_id << "\">"
	   << sgml::escapeString(getParam("symbol"))
	   << "</glossterm>";
	return 0;
}


docstring InsetNomencl::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}


int InsetNomencl::docbookGlossary(odocstream & os) const
{
	os << "<glossentry id=\"" << nomenclature_entry_id << "\">\n"
	   << "<glossterm>"
	   << sgml::escapeString(getParam("symbol"))
	   << "</glossterm>\n"
	   << "<glossdef><para>"
	   << sgml::escapeString(getParam("description"))
	   << "</para></glossdef>\n"
	   <<"</glossentry>\n";
	return 4;
}


void InsetNomencl::validate(LaTeXFeatures & features) const
{
	features.require("nomencl");
}


void InsetNomencl::addToToc(DocIterator const & cpit, bool output_active,
							UpdateType) const
{
	docstring const str = getParam("symbol");
	buffer().tocBackend().toc("nomencl")->push_back(TocItem(cpit, 0, str, output_active));
}


/////////////////////////////////////////////////////////////////////
//
// InsetPrintNomencl
//
/////////////////////////////////////////////////////////////////////

InsetPrintNomencl::InsetPrintNomencl(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetPrintNomencl::findInfo(string const & /* cmdName */)
{
	// The symbol width is set via nomencl's \nomlabelwidth in 
	// InsetPrintNomencl::latex and not as optional parameter of
	// \printnomenclature
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		// how is the width set?
		// values: none|auto|custom
		param_info_.add("set_width", ParamInfo::LYX_INTERNAL);
		// custom width
		param_info_.add("width", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


docstring InsetPrintNomencl::screenLabel() const
{
	return _("Nomenclature");
}


struct NomenclEntry {
	NomenclEntry() : par(0) {}
	NomenclEntry(docstring s, docstring d, Paragraph const * p)
	  : symbol(s), desc(d), par(p)
	{}

	docstring symbol;
	docstring desc;
	Paragraph const * par;
};


typedef map<docstring, NomenclEntry > EntryMap;


docstring InsetPrintNomencl::xhtml(XHTMLStream &, OutputParams const & op) const
{
	shared_ptr<Toc const> toc = buffer().tocBackend().toc("nomencl");

	EntryMap entries;
	Toc::const_iterator it = toc->begin();
	Toc::const_iterator const en = toc->end();
	for (; it != en; ++it) {
		DocIterator dit = it->dit();
		Paragraph const & par = dit.innerParagraph();
		Inset const * inset = par.getInset(dit.top().pos());
		if (!inset)
			return docstring();
		InsetCommand const * ic = inset->asInsetCommand();
		if (!ic)
			return docstring();
		
		// FIXME We need a link to the paragraph here, so we
		// need some kind of struct.
		docstring const symbol = ic->getParam("symbol");
		docstring const desc = ic->getParam("description");
		docstring const prefix = ic->getParam("prefix");
		docstring const sortas = prefix.empty() ? symbol : prefix;
		
		entries[sortas] = NomenclEntry(symbol, desc, &par);
	}
	
	if (entries.empty())
		return docstring();
	
	// we'll use our own stream, because we are going to defer everything.
	// that's how we deal with the fact that we're probably inside a standard
	// paragraph, and we don't want to be.
	odocstringstream ods;
	XHTMLStream xs(ods);
	
	InsetLayout const & il = getLayout();
	string const & tag = il.htmltag();
	docstring toclabel = translateIfPossible(from_ascii("Nomenclature"),
		op.local_font->language()->lang());

	xs << html::StartTag("div", "class='nomencl'")
	   << html::StartTag(tag, "class='nomencl'")
		 << toclabel 
		 << html::EndTag(tag)
	   << html::CR()
	   << html::StartTag("dl")
	   << html::CR();
	
	EntryMap::const_iterator eit = entries.begin();
	EntryMap::const_iterator const een = entries.end();
	for (; eit != een; ++eit) {
		NomenclEntry const & ne = eit->second;
		string const parid = ne.par->magicLabel();
		xs << html::StartTag("dt")
		   << html::StartTag("a", "href='#" + parid + "' class='nomencl'")
		   << ne.symbol
		   << html::EndTag("a")
		   << html::EndTag("dt")
		   << html::CR()
		   << html::StartTag("dd")
		   << ne.desc
		   << html::EndTag("dd")
		   << html::CR();
	}

	xs << html::EndTag("dl")
	   << html::CR()
	   << html::EndTag("div")
	   << html::CR();

	return ods.str();
}


void InsetPrintNomencl::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(NOMENCL_PRINT_CODE);
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


bool InsetPrintNomencl::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {

	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_INSET_MODIFY:
		status.setEnabled(true);
		return true;

	default:
		return InsetCommand::getStatus(cur, cmd, status);
	}
}


// FIXME This should be changed to use the TOC. Perhaps
// that could be done when XHTML output is added.
int InsetPrintNomencl::docbook(odocstream & os, OutputParams const &) const
{
	os << "<glossary>\n";
	int newlines = 2;
	InsetIterator it = inset_iterator_begin(buffer().inset());
	while (it) {
		if (it->lyxCode() == NOMENCL_CODE) {
			newlines += static_cast<InsetNomencl const &>(*it).docbookGlossary(os);
			++it;
		} else if (!it->producesOutput()) {
			// Ignore contents of insets that are not in output
			size_t const depth = it.depth();
			++it;
			while (it.depth() > depth)
				++it;
		} else {
			++it;
		}
	}
	os << "</glossary>\n";
	return newlines;
}


namespace {
docstring nomenclWidest(Buffer const & buffer, OutputParams const & runparams)
{
	// nomenclWidest() determines and returns the widest used
	// nomenclature symbol in the document

	int w = 0;
	docstring symb;
	InsetNomencl const * nomencl = 0;
	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->insetList().empty())
			continue;
		InsetList::const_iterator iit = it->insetList().begin();
		InsetList::const_iterator eend = it->insetList().end();
		for (; iit != eend; ++iit) {
			Inset * inset = iit->inset;
			if (inset->lyxCode() != NOMENCL_CODE)
				continue;
			nomencl = static_cast<InsetNomencl const *>(inset);
			docstring const symbol =
				nomencl->getParam("symbol");
			// This is only an approximation,
			// but the best we can get.
			int const wx = use_gui ?
				theFontMetrics(Font()).width(symbol) :
				symbol.size();
			if (wx > w) {
				w = wx;
				symb = symbol;
			}
		}
	}
	// return the widest (or an empty) string
	if (symb.empty())
		return symb;

	// we have to encode the string properly
	pair<docstring, docstring> latex_symb =
		runparams.encoding->latexString(symb, runparams.dryrun);
	if (!latex_symb.second.empty())
		LYXERR0("Omitting uncodable characters '"
			<< latex_symb.second
			<< "' in nomencl widest string!");
	return latex_symb.first;
}
} // namespace anon


void InsetPrintNomencl::latex(otexstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams = runparams_in;
	if (getParam("set_width") == "auto") {
		docstring widest = nomenclWidest(buffer(), runparams);
		// Set the label width via nomencl's command \nomlabelwidth.
		// This must be output before the command \printnomenclature
		if (!widest.empty()) {
			os << "\\settowidth{\\nomlabelwidth}{"
			   << widest
			   << "}\n";
		}
	} else if (getParam("set_width") == "custom") {
		// custom length as optional arg of \printnomenclature
		string const width =
			Length(to_ascii(getParam("width"))).asLatexString();
		os << '\\'
		   << from_ascii(getCmdName())
		   << '['
		   << from_ascii(width)
		   << "]{}";
		return;
	}
	// output the command \printnomenclature
	os << getCommand(runparams);
}


void InsetPrintNomencl::validate(LaTeXFeatures & features) const
{
	features.require("nomencl");
}


InsetCode InsetPrintNomencl::lyxCode() const
{
	return NOMENCL_PRINT_CODE;
}


string InsetPrintNomencl::contextMenuName() const
{
	return "context-nomenclprint";
}


} // namespace lyx
