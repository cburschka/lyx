/**
 * \file InsetIndexMacro.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetIndexMacro.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "Dimension.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "FontInfo.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "InsetList.h"
#include "LaTeX.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "xml.h"
#include "texstream.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Translator.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

typedef Translator<string, InsetIndexMacroParams::Type> InsetIndexMacroTranslator;
typedef Translator<docstring, InsetIndexMacroParams::Type> InsetIndexMacroTranslatorLoc;

InsetIndexMacroTranslator const init_insetindexmacrotranslator()
{
	InsetIndexMacroTranslator translator("see", InsetIndexMacroParams::See);
	translator.addPair("seealso", InsetIndexMacroParams::Seealso);
	translator.addPair("subentry", InsetIndexMacroParams::Subentry);
	translator.addPair("sortkey", InsetIndexMacroParams::Sortkey);
	return translator;
}


InsetIndexMacroTranslatorLoc const init_insetindexmacrotranslator_loc()
{
	InsetIndexMacroTranslatorLoc translator(_("See"), InsetIndexMacroParams::See);
	translator.addPair(_("See also"), InsetIndexMacroParams::Seealso);
	translator.addPair(_("Subentry"), InsetIndexMacroParams::Subentry);
	translator.addPair(_("Sort as"), InsetIndexMacroParams::Sortkey);
	return translator;
}


InsetIndexMacroTranslator const & insetindexmacrotranslator()
{
	static InsetIndexMacroTranslator const macrotranslator =
			init_insetindexmacrotranslator();
	return macrotranslator;
}


InsetIndexMacroTranslatorLoc const & insetindexmacrotranslator_loc()
{
	static InsetIndexMacroTranslatorLoc const translator =
			init_insetindexmacrotranslator_loc();
	return translator;
}

} // namespace


InsetIndexMacroParams::InsetIndexMacroParams()
	: type(See)
{}


void InsetIndexMacroParams::write(ostream & os) const
{
	string const label = insetindexmacrotranslator().find(type);
	os << "IndexMacro " << label << "\n";
}


void InsetIndexMacroParams::read(Lexer & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = insetindexmacrotranslator().find(label);
}


/////////////////////////////////////////////////////////////////////
//
// InsetIndexMacro
//
/////////////////////////////////////////////////////////////////////

InsetIndexMacro::InsetIndexMacro(Buffer * buf, string const & label)
	: InsetCollapsible(buf)
{
	setDrawFrame(true);
	setFrameColor(Color_insetframe);
	params_.type = insetindexmacrotranslator().find(label);
}


InsetIndexMacro::~InsetIndexMacro()
{}


docstring InsetIndexMacro::layoutName() const
{
	return from_ascii("IndexMacro:" + insetindexmacrotranslator().find(params_.type));
}

InsetCode InsetIndexMacro::lyxCode() const
{
	return params_.type == InsetIndexMacroParams::Sortkey
			? INDEXMACRO_SORTKEY_CODE
			: INDEXMACRO_CODE;
}


void InsetIndexMacro::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsible::write(os);
}


void InsetIndexMacro::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsible::read(lex);
}


void InsetIndexMacro::getLatex(otexstream & os, OutputParams const & runparams) const
{
	if (params_.type == InsetIndexMacroParams::Subentry) {
		if (hasSortKey()) {
			getSortkey(os, runparams);
			os << "@";
			InsetText::latex(os, runparams);
		} else {
			odocstringstream ourlatex;
			otexstream ots(ourlatex);
			InsetText::latex(ots, runparams);
			odocstringstream ourplain;
			InsetText::plaintext(ourplain, runparams);
			// These are the LaTeX and plaintext representations
			docstring latexstr = ourlatex.str();
			docstring plainstr = ourplain.str();
			processLatexSorting(os, runparams, latexstr, plainstr);
		}
		return;
	}

	if (params_.type == InsetIndexMacroParams::See)
		os << "see{";
	else if (params_.type == InsetIndexMacroParams::Seealso)
		os << "seealso{";

	InsetCollapsible::latex(os, runparams);

	if (params_.type == InsetIndexMacroParams::See
	     || params_.type == InsetIndexMacroParams::Seealso)
		os << "}";
}


int InsetIndexMacro::getPlaintext(odocstringstream & os,
			    OutputParams const & runparams, size_t max_length) const
{
	return InsetText::plaintext(os, runparams, max_length);
}


void InsetIndexMacro::getDocbook(XMLStream & xs, OutputParams const & runparams) const
{
	InsetText::docbook(xs, runparams);
}


docstring InsetIndexMacro::getXhtml(XMLStream & xs, OutputParams const & runparams) const
{
	return InsetText::xhtml(xs, runparams);
}


bool InsetIndexMacro::hasNoContent() const
{
	return paragraphs().front().empty();
}


void InsetIndexMacro::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) == "changetype") {
			cur.recordUndoInset(this);
			params_.type = insetindexmacrotranslator().find(cmd.getArg(1));
			break;
		}
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}

	default:
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}
}


bool InsetIndexMacro::getStatus(Cursor & cur, FuncRequest const & cmd,
				FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			docstring const newtype = from_utf8(cmd.getArg(1));
			bool const enabled = (params_.type == InsetIndexMacroParams::See
					      || params_.type == InsetIndexMacroParams::Seealso)
					    && (newtype == "see" || newtype == "seealso");
			flag.setEnabled(enabled);
			flag.setOnOff(
				newtype == from_ascii(insetindexmacrotranslator().find(params_.type)));
			return true;
		}
		return InsetCollapsible::getStatus(cur, cmd, flag);

	default:
		return InsetCollapsible::getStatus(cur, cmd, flag);
	}
}


void InsetIndexMacro::processLatexSorting(otexstream & os, OutputParams const & runparams,
				docstring const latex, docstring const plain) const
{
	if (contains(latex, '\\') && !contains(latex, '@')) {
		// Plaintext might return nothing (e.g. for ERTs).
		// In that case, we use LaTeX.
		docstring const spart = (plain.empty()) ? latex : plain;
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
						      "problems with the sub-entry '%1$s'.\n"
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


docstring InsetIndexMacro::toolTip(BufferView const &, int, int) const
{
	return insetindexmacrotranslator_loc().find(params_.type);
}


string InsetIndexMacro::params2string(InsetIndexMacroParams const & params)
{
	ostringstream data;
	data << "IndexMacro" << ' ';
	params.write(data);
	return data.str();
}


void InsetIndexMacro::string2params(string const & in, InsetIndexMacroParams & params)
{
	params = InsetIndexMacroParams();

	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetIndexMacro::string2params");
	lex >> "IndexMacro" >> "see";

	params.read(lex);
}


bool InsetIndexMacro::hasSortKey() const
{
	Paragraph const & par = paragraphs().front();
	InsetList::const_iterator it = par.insetList().begin();
	for (; it != par.insetList().end(); ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == INDEXMACRO_SORTKEY_CODE)
			return true;
	}
	return false;
}


void InsetIndexMacro::getSortkey(otexstream & os, OutputParams const & runparams) const
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


string InsetIndexMacro::contextMenuName() const
{
	return "context-indexmacro";
}


string InsetIndexMacro::contextMenu(BufferView const & bv, int x, int y) const
{
	// We override the implementation of InsetCollapsible,
	// because we have eytra entries.
	string owncm = "context-edit-index;";
	return owncm + InsetCollapsible::contextMenu(bv, x, y);
}


bool InsetIndexMacro::insetAllowed(InsetCode code) const
{
	switch (code) {
	case INDEX_CODE:
		return false;
	case INDEXMACRO_SORTKEY_CODE:
		return (params_.type == InsetIndexMacroParams::Subentry
			&& !hasSortKey());
	default:
		return InsetCollapsible::insetAllowed(code);
	}
}

} // namespace lyx
