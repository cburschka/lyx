/**
 * \file insetindex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "insetindex.h"

#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "metricsinfo.h"
#include "sgml.h"

#include "support/std_ostream.h"

using std::string;
using std::ostream;


InsetIndex::InsetIndex(InsetCommandParams const & p)
	: InsetCommand(p, "index")
{}


// InsetIndex::InsetIndex(InsetCommandParams const & p, bool)
//	: InsetCommand(p, false)
// {}


string const InsetIndex::getScreenLabel(Buffer const &) const
{
	return _("Idx");
}


int InsetIndex::docbook(Buffer const &, ostream & os,
			OutputParams const &) const
{
	os << "<indexterm><primary>" << sgml::escapeString(getContents())
	   << "</primary></indexterm>";
	return 0;
}


InsetBase::Code InsetIndex::lyxCode() const
{
	return InsetBase::INDEX_CODE;
}



InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


// InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p, bool)
//	: InsetCommand(p, false)
// {}


string const InsetPrintIndex::getScreenLabel(Buffer const &) const
{
	return _("Index");
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.require("makeidx");
}


InsetBase::Code InsetPrintIndex::lyxCode() const
{
	return InsetBase::INDEX_PRINT_CODE;
}
