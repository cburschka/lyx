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
{
	setLayout(bp);
}


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


void InsetIndex::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.realize(tmpfont);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
}


void InsetIndex::draw(PainterInfo & pi, int x, int y) const
{
	Font tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	pi.base.font.realize(tmpfont);
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetIndex::getDrawFont(Font & font) const
{
	font = Font(Font::ALL_INHERIT);
	font.realize(layout_.font);
}


bool InsetIndex::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// paragraph breaks not allowed
		case LFUN_BREAK_PARAGRAPH:
		case LFUN_BREAK_PARAGRAPH_SKIP:
			status.enabled(false);
			return true;

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
		}
}



InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p, string())
{}


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
