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

#include "support/gettext.h"

#include <ostream>

using namespace std;

namespace lyx {


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


void InsetIndex::write(Buffer const & buf, ostream & os) const
{
	os << to_utf8(name()) << "\n";
	InsetCollapsable::write(buf, os);
}


void InsetIndex::addToToc(Buffer const & buf,
	ParConstIterator const & cpit) const
{
	ParConstIterator pit = cpit;
	pit.push_back(*this);

	Toc & toc = buf.tocBackend().toc("index");
	docstring str;
	str = getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
}


InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


CommandInfo const * InsetPrintIndex::findInfo(string const & /* cmdName */)
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
