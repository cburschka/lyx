/**
 * \file insettoc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettoc.h"

#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "metricsinfo.h"
#include "toc.h"

#include "support/std_ostream.h"

using std::string;
using std::ostream;



InsetTOC::InsetTOC(InsetCommandParams const & p)
	: InsetCommand(p)
{}


InsetTOC::~InsetTOC()
{
	InsetCommandMailer("toc", *this).hideDialog();
}


std::auto_ptr<InsetBase> InsetTOC::clone() const
{
	return std::auto_ptr<InsetBase>(new InsetTOC(*this));
}


string const InsetTOC::getScreenLabel(Buffer const &) const
{
	if (getCmdName() == "tableofcontents")
		return _("Table of Contents");
	return _("Unknown toc list");
}


InsetOld::Code InsetTOC::lyxCode() const
{
	if (getCmdName() == "tableofcontents")
		return InsetOld::TOC_CODE;
	return InsetOld::NO_CODE;
}


void InsetTOC::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCommand::metrics(mi, dim);
	int const x1 = (mi.base.textwidth - dim.wid) / 2;
	button().setBox(Box(x1, x1 + dim.wid, -dim.asc, dim.des));
	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetTOC::draw(PainterInfo & pi, int x, int y) const
{
	InsetCommand::draw(pi, button().box().x1, y);
}


DispatchResult
InsetTOC::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
		if (button().box().contains(cmd.x, cmd.y))
			InsetCommandMailer("toc", *this).showDialog(cmd.view());
		return DISPATCHED;

	case LFUN_INSET_DIALOG_SHOW:
		InsetCommandMailer("toc", *this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return InsetCommand::priv_dispatch(cmd, idx, pos);
	}
}


int InsetTOC::ascii(Buffer const & buffer, ostream & os, int) const
{
	os << getScreenLabel(buffer) << "\n\n";

	lyx::toc::asciiTocList(lyx::toc::getType(getCmdName()), buffer, os);

	os << "\n";
	return 0;
}


int InsetTOC::linuxdoc(Buffer const &, ostream & os) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc>";
	return 0;
}


int InsetTOC::docbook(Buffer const &, ostream & os, bool) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}
