/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcommand.h"
#include "debug.h"
#include "frontends/Painter.h"

using std::ostream;
using std::endl;


InsetCommand::InsetCommand(InsetCommandParams const & p, bool)
	: p_(p.getCmdName(), p.getContents(), p.getOptions())
{}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_.setCmdName(p.getCmdName());
	p_.setContents(p.getContents());
	p_.setOptions(p.getOptions());
}


int InsetCommand::latex(Buffer const *, ostream & os,
			bool /*fragile*/, bool/*fs*/) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::ascii(Buffer const *, ostream &, int) const
{
	return 0;
}


int InsetCommand::linuxdoc(Buffer const *, ostream &) const
{
	return 0;
}


int InsetCommand::docbook(Buffer const *, ostream &, bool) const
{
	return 0;
}
