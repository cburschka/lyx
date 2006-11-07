/**
 * \file insetnomencl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "insetnomencl.h"

#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "metricsinfo.h"
#include "sgml.h"


namespace lyx {

using std::string;


InsetNomencl::InsetNomencl(InsetCommandParams const & p)
	: InsetCommand(p, "nomenclature")
{}


docstring const InsetNomencl::getScreenLabel(Buffer const &) const
{
	return _("Glo");
}


int InsetNomencl::docbook(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	// FIXME: This does not work, because the entry needs to be put
	// in the glossary.
#if 0
	os << "<glossentry><glossterm>"
	   << sgml::escapeString(getParam("symbol"))
	   << "</glossterm><glossdef><para>"
	   << sgml::escapeString(getParam("description"))
	   << "</para></glossdef></glossentry>";
#endif
	return 0;
}


void InsetNomencl::validate(LaTeXFeatures & features) const
{
	features.require("nomencl");
}


InsetBase::Code InsetNomencl::lyxCode() const
{
	return InsetBase::NOMENCL_CODE;
}


InsetPrintNomencl::InsetPrintNomencl(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


docstring const InsetPrintNomencl::getScreenLabel(Buffer const &) const
{
	return _("Glossary");
}


int InsetPrintNomencl::docbook(Buffer const &, odocstream & os,
		OutputParams const &) const
{
	// FIXME This does not work, we need to define the entries here.
	//os << "<glossary><glossary/>";
	return 0;
}


void InsetPrintNomencl::validate(LaTeXFeatures & features) const
{
	features.require("nomencl");
}


InsetBase::Code InsetPrintNomencl::lyxCode() const
{
	return InsetBase::NOMENCL_PRINT_CODE;
}


} // namespace lyx
