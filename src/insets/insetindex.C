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

#include "support/std_ostream.h"

using std::string;
using std::ostream;


InsetIndex::InsetIndex(InsetCommandParams const & p)
	: InsetCommand(p)
{}


// InsetIndex::InsetIndex(InsetCommandParams const & p, bool)
//	: InsetCommand(p, false)
// {}


InsetIndex::~InsetIndex()
{
	InsetCommandMailer mailer("index", *this);
	mailer.hideDialog();
}


string const InsetIndex::getScreenLabel(Buffer const &) const
{
	return _("Idx");
}


void InsetPrintIndex::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCommand::metrics(mi, dim);
	center_indent_ = (mi.base.textwidth - dim.wid) / 2;
	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetPrintIndex::draw(PainterInfo & pi, int x, int y) const
{
	InsetCommand::draw(pi, x + center_indent_, y);
}


DispatchResult
InsetIndex::priv_dispatch(FuncRequest const & cmd,
			  idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
		case LFUN_INSET_EDIT:
			InsetCommandMailer("index", *this).showDialog(cmd.view());
			return DispatchResult(true, true);

		default:
			return InsetCommand::priv_dispatch(cmd, idx, pos);
	}
}


int InsetIndex::docbook(Buffer const &, ostream & os,
			LatexRunParams const &) const
{
	os << "<indexterm><primary>" << getContents()
	   << "</primary></indexterm>";
	return 0;
}


InsetOld::Code InsetIndex::lyxCode() const
{
	return InsetOld::INDEX_CODE;
}



InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p)
	: InsetCommand(p)
{}


// InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p, bool)
//	: InsetCommand(p, false)
// {}


InsetPrintIndex::~InsetPrintIndex()
{}


string const InsetPrintIndex::getScreenLabel(Buffer const &) const
{
	return _("Index");
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.require("makeidx");
}


InsetOld::Code InsetPrintIndex::lyxCode() const
{
	return InsetOld::INDEX_PRINT_CODE;
}
