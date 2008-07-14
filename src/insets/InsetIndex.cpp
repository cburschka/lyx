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
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "sgml.h"
#include "TocBackend.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

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
	int i = 7;
	odocstringstream ods;
	InsetText::latex(ods, runparams);
	odocstringstream ods2;
	InsetText::plaintext(ods2, runparams);
	std::vector<docstring> const levels =
		getVectorFromString(ods.str(), from_ascii("!"));
	std::vector<docstring> const levels_plain =
		getVectorFromString(ods2.str(), from_ascii("!"));
	vector<docstring>::const_iterator it = levels.begin();
	vector<docstring>::const_iterator end = levels.end();
	vector<docstring>::const_iterator it2 = levels_plain.begin();
	for (; it != end; ++it) {
		if (it > levels.begin()) {
			os << '!';
			i += 1;
		}
		// correctly sort macros and formatted strings
		// if we do find a command, prepend a plain text
		// version of the content to get sorting right,
		// e.g. \index{LyX@\LyX}, \index{text@\textbf{text}}
		// Don't do that if the user entered '@' himself, though.
		if (contains(*it, '\\') && !contains(*it, '@')) {
			// remove remaining \'s for the sorting part
			docstring const ppart =
				subst(*it2, from_ascii("\\"), docstring());
			os << ppart;
			os << '@';
			i += ppart.size() + 1;
		}
		docstring const tpart = *it;
		os << tpart;
		i += tpart.size();
		if (it2 < levels_plain.end())
			++it2;
	}
	os << '}';
	i += 1;
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

	Toc & toc = buffer().tocBackend().toc("index");
	docstring str;
	str = getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
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
