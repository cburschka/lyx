/**
 * \file insetcommand.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetcommand.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "lyxlex.h"

#include "frontends/Painter.h"

#include "Lsstream.h"

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


dispatch_result InsetCommand::localDispatch(FuncRequest const & cmd)
{
	InsetCommandParams p;
	InsetCommandMailer::string2params(cmd.argument, p);
	if (p.getCmdName().empty())
		return UNDISPATCHED;

	setParams(p);
	if (view())
		view()->updateInset(this, true);

	return DISPATCHED;
}

InsetCommandMailer::InsetCommandMailer(string const & name,
				       InsetCommand & inset)
	: name_(name), inset_(inset)
{}


string const InsetCommandMailer::inset2string() const
{
	return params2string(inset_.params());
}


void InsetCommandMailer::string2params(string const & in,
				       InsetCommandParams & params)
{
	params.setCmdName(string());
	params.setContents(string());
	params.setOptions(string());

	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	params.read(lex);
}


string const
InsetCommandMailer::params2string(InsetCommandParams const & params)
{
	ostringstream data;
	params.write(data);
	data << "\\end_inset\n";

	return data.str();
}
