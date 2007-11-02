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

#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "sgml.h"

#include "support/std_ostream.h"


namespace lyx {

using std::string;
using std::ostream;


InsetIndex::InsetIndex(BufferParams const & bp)
	: InsetCollapsable(bp)
{}


InsetIndex::InsetIndex(InsetIndex const & in)
	: InsetCollapsable(in)
{}


int InsetIndex::docbook(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	os << "<indexterm><primary>";
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</primary></indexterm>";
	return i;
}


Inset * InsetIndex::clone() const
{
	return new InsetIndex(*this);
}


void InsetIndex::write(Buffer const & buf, std::ostream & os) const
{
	os << to_utf8(name()) << "\n";
	InsetCollapsable::write(buf, os);
}


InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


CommandInfo const * InsetPrintIndex::findInfo(std::string const & /* cmdName */)
{
	static const char * const paramnames[] = {"name", ""};
	static const bool isoptional[] = {false};
	static const CommandInfo info = {1, paramnames, isoptional};
	return &info;
}


docstring const InsetPrintIndex::getScreenLabel(Buffer const &) const
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
