/**
 * \file insetcommand.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcommand.h"

#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxlex.h"
#include "metricsinfo.h"

#include "support/std_sstream.h"


using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;


InsetCommand::InsetCommand(InsetCommandParams const & p)
	: p_(p.getCmdName(), p.getContents(), p.getOptions()),
	  set_label_(false)
{}


void InsetCommand::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (!set_label_) {
		set_label_ = true;
		button_.update(getScreenLabel(*mi.base.bv->buffer()),
			       editable() != NOT_EDITABLE);
	}
	button_.metrics(mi, dim);
	dim_ = dim;
}


void InsetCommand::draw(PainterInfo & pi, int x, int y) const
{
	button_.draw(pi, x, y);
}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_ = p;
	set_label_ = false;
}


int InsetCommand::latex(Buffer const &, ostream & os,
			OutputParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::plaintext(Buffer const &, ostream &,
			OutputParams const &) const
{
	return 0;
}


int InsetCommand::linuxdoc(Buffer const &, ostream &,
			   OutputParams const &) const
{
	return 0;
}


int InsetCommand::docbook(Buffer const &, ostream &,
			  OutputParams const &) const
{
	return 0;
}


DispatchResult
InsetCommand::priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(cmd.argument, p);
		if (p.getCmdName().empty())
			return DispatchResult(false);

		setParams(p);
		cmd.view()->updateInset(this);
		return DispatchResult(true, true);
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetCommandMailer(cmd.argument, *this).updateDialog(cmd.view());
		return DispatchResult(true, true);

	case LFUN_MOUSE_RELEASE:
		edit(cmd.view(), true);
		return DispatchResult(true);

	default:
		return DispatchResult(false);
	}

}


InsetCommandMailer::InsetCommandMailer(string const & name,
				       InsetCommand & inset)
	: name_(name), inset_(inset)
{}


string const InsetCommandMailer::inset2string(Buffer const &) const
{
	return params2string(name(), inset_.params());
}


void InsetCommandMailer::string2params(string const & in,
				       InsetCommandParams & params)
{
	params = InsetCommandParams();

	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const name = lex.getString();
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "LatexCommand")
			return;
	}
	if (lex.isOK()) {
		params.read(lex);
	}
}


string const InsetCommandMailer::params2string(string const & name,
				  InsetCommandParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);
	data << "\\end_inset\n";
	return data.str();
}
