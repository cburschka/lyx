/**
 * \file insetindex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>


#include "insetindex.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "support/LOstream.h"

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


string const InsetIndex::getScreenLabel(Buffer const *) const
{
	return _("Idx");
}


dispatch_result InsetIndex::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {
		case LFUN_INSET_EDIT:
			InsetCommandMailer("index", *this).showDialog(cmd.view());
			return DISPATCHED;

		default:
			return InsetCommand::localDispatch(cmd);
	}
}


int InsetIndex::docbook(Buffer const *, ostream & os, bool) const
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


string const InsetPrintIndex::getScreenLabel(Buffer const *) const
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
