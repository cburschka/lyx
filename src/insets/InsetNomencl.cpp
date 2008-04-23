/**
 * \file InsetNomencl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetNomencl.h"
#include "InsetNote.h"

#include "Buffer.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "support/gettext.h"
#include "InsetIterator.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "sgml.h"

#include "support/docstream.h"

using namespace std;

namespace lyx {


/////////////////////////////////////////////////////////////////////
//
// InsetPrintNomencl
//
/////////////////////////////////////////////////////////////////////

InsetNomencl::InsetNomencl(InsetCommandParams const & p)
	: InsetCommand(p, "nomenclature"),
	  nomenclature_entry_id(sgml::uniqueID(from_ascii("nomen")))
{}


ParamInfo const & InsetNomencl::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("prefix", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("symbol", ParamInfo::LATEX_REQUIRED);
		param_info_.add("description", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


docstring InsetNomencl::screenLabel() const
{
	return _("Nom");
}


int InsetNomencl::docbook(odocstream & os, OutputParams const &) const
{
	os << "<glossterm linkend=\"" << nomenclature_entry_id << "\">"
	   << sgml::escapeString(getParam("symbol"))
	   << "</glossterm>";
	return 0;
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


/////////////////////////////////////////////////////////////////////
//
// InsetPrintNomencl
//
/////////////////////////////////////////////////////////////////////

InsetPrintNomencl::InsetPrintNomencl(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


ParamInfo const & InsetPrintNomencl::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("labelwidth", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


docstring InsetPrintNomencl::screenLabel() const
{
	return _("Nomenclature");
}


int InsetPrintNomencl::docbook(odocstream & os, OutputParams const &) const
{
	os << "<glossary>\n";
	int newlines = 2;
	for (InsetIterator it = inset_iterator_begin(buffer().inset()); it;) {
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


void InsetPrintNomencl::validate(LaTeXFeatures & features) const
{
	features.require("nomencl");
}


InsetCode InsetPrintNomencl::lyxCode() const
{
	return NOMENCL_PRINT_CODE;
}


} // namespace lyx
