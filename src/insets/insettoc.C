/**
 * \file insettoc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>


#include "gettext.h"
#include "insettoc.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "debug.h"
#include "toc.h"

using std::vector;
using std::ostream;


InsetTOC::InsetTOC(InsetCommandParams const & p, bool same_id)
	: InsetCommand(p, same_id)
{}


InsetTOC::~InsetTOC()
{
	InsetCommandMailer mailer("toc", *this);
	mailer.hideDialog();
}


string const InsetTOC::getScreenLabel(Buffer const *) const
{
	string const cmdname(getCmdName());

	if (cmdname == "tableofcontents")
		return _("Table of Contents");
	return _("Unknown toc list");
}


Inset::Code InsetTOC::lyxCode() const
{
	string const cmdname(getCmdName());
	if (cmdname == "tableofcontents")
		return Inset::TOC_CODE;
	return Inset::NO_CODE;
}


void InsetTOC::edit(BufferView *, int, int, mouse_button::state)
{
	InsetCommandMailer mailer("toc", *this);
	mailer.showDialog();
}


void InsetTOC::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetTOC::ascii(Buffer const * buffer, ostream & os, int) const
{
	os << getScreenLabel(buffer) << "\n\n";

	toc::asciiTocList(toc::getType(getCmdName()), buffer, os);

	os << "\n";
	return 0;
}


int InsetTOC::linuxdoc(Buffer const *, ostream & os) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc>";
	return 0;
}


int InsetTOC::docbook(Buffer const *, ostream & os, bool) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}
