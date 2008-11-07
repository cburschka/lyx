/**
 * \file InsetIndex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetIndex.h"

#include "Buffer.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "sgml.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetIndex
//
///////////////////////////////////////////////////////////////////////


InsetIndex::InsetIndex(Buffer const & buf)
	: InsetCollapsable(buf)
{}


int InsetIndex::latex(odocstream & os,
		      OutputParams const & runparams) const
{
	os << "\\index";
	os << '{';
	int i = 0;

	// get contents of InsetText as LaTeX and plaintext
	odocstringstream ourlatex;
	InsetText::latex(ourlatex, runparams);
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
			docstring spart2;
			for (size_t n = 0; n < spart.size(); ++n) {
				try {
					spart2 += runparams.encoding->latexChar(spart[n]);
				} catch (EncodingException & /* e */) {
					LYXERR0("Uncodable character in index entry. Sorting might be wrong!");
				}
			}
			if (spart != spart2 && !runparams.dryrun) {
				// FIXME: warning should be passed to the error dialog
				frontend::Alert::warning(_("Index sorting failed"),
				bformat(_("LyX's automatic index sorting algorithm faced\n"
				  "problems with the entry '%1$s'.\n"
				  "Please specify the sorting of this entry manually, as\n"
				  "explained in the User Guide."), spart));
			}
			// remove remaining \'s for the sorting part
			docstring const ppart =
				subst(spart2, from_ascii("\\"), docstring());
			os << ppart;
			os << '@';
		}
		docstring const tpart = *it;
		os << tpart;
		if (it2 < levels_plain.end())
			++it2;
	}
	// write the bit that followed "|"
	if (!cmd.empty())
		os << "|" << cmd;
	os << '}';
	return i;
}


int InsetIndex::docbook(odocstream & os, OutputParams const & runparams) const
{
	os << "<indexterm><primary>";
	int const i = InsetText::docbook(os, runparams);
	os << "</primary></indexterm>";
	return i;
}


void InsetIndex::write(ostream & os) const
{
	os << to_utf8(name()) << "\n";
	InsetCollapsable::write(os);
}


void InsetIndex::addToToc(DocIterator const & cpit)
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(*this));
	docstring const item = text().asString(0, 1, AS_STR_LABEL | AS_STR_INSETS);
	buffer().tocBackend().toc("index").push_back(TocItem(pit, 0, item));
	// Proceed with the rest of the inset.
	InsetCollapsable::addToToc(cpit);
}


/////////////////////////////////////////////////////////////////////
//
// InsetPrintIndex
//
///////////////////////////////////////////////////////////////////////

InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


ParamInfo const & InsetPrintIndex::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty())
		param_info_.add("name", ParamInfo::LATEX_REQUIRED);
	return param_info_;
}


docstring InsetPrintIndex::screenLabel() const
{
	return _("Index");
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.require("makeidx");
}


InsetCode InsetPrintIndex::lyxCode() const
{
	return INDEX_PRINT_CODE;
}

} // namespace lyx
