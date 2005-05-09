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
#include "FuncStatus.h"
#include "lyxlex.h"
#include "metricsinfo.h"

#include <sstream>


using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;


InsetCommand::InsetCommand(InsetCommandParams const & p,
			   string const & mailer_name)
	: p_(p.getCmdName(), p.getContents(), p.getOptions(), p.getSecOptions()),
	  mailer_name_(mailer_name),
	  set_label_(false)
{}


InsetCommand::~InsetCommand()
{
	if (!mailer_name_.empty())
		InsetCommandMailer(mailer_name_, *this).hideDialog();
}


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
	setPosCache(pi, x, y);
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


void InsetCommand::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_REFRESH:
		set_label_ = false;
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(mailer_name_, cmd.argument, p);
		if (p.getCmdName().empty())
			cur.noUpdate();
		else
			setParams(p);
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetCommandMailer(cmd.argument, *this).updateDialog(&cur.bv());
		break;

	case LFUN_INSET_DIALOG_SHOW:
	case LFUN_MOUSE_RELEASE: {
		if (!mailer_name_.empty())
			InsetCommandMailer(mailer_name_, *this).showDialog(&cur.bv());
		break;
	}

	default:
		InsetBase::doDispatch(cur, cmd);
		break;
	}

}


bool InsetCommand::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	// suppress these
	case LFUN_INSET_ERT:
		status.enabled(false);
		return true;
	// we handle these
	case LFUN_INSET_REFRESH:
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_INSET_DIALOG_SHOW:
		status.enabled(true);
		return true;
	default:
		return InsetBase::getStatus(cur, cmd, status);
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


void InsetCommandMailer::string2params(string const & name,
				       string const & in,
				       InsetCommandParams & params)
{
	params = InsetCommandParams();
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string n;
	lex >> n;
	if (!lex || n != name)
		return print_mailer_error("InsetCommandMailer", in, 1, name);

	// This is part of the inset proper that is usually swallowed
	// by LyXText::readInset
	string id;
	lex >> id;
	if (!lex || id != "LatexCommand")
		return print_mailer_error("InsetCommandMailer", in, 2, "LatexCommand");

	params.read(lex);
}


string const
InsetCommandMailer::params2string(string const & name,
				  InsetCommandParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);
	data << "\\end_inset\n";
	return data.str();
}
