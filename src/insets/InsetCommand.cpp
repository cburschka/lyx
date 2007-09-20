/**
 * \file InsetCommand.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCommand.h"

#include "Buffer.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Lexer.h"
#include "MetricsInfo.h"

#include <sstream>


namespace lyx {

using std::string;
using std::istringstream;
using std::ostringstream;


InsetCommand::InsetCommand(InsetCommandParams const & p,
			   string const & mailer_name)
	: p_(p),
	  mailer_name_(mailer_name),
	  mouse_hover_(false),
	  updateButtonLabel_(true)
{}


InsetCommand::~InsetCommand()
{
	if (!mailer_name_.empty())
		InsetCommandMailer(mailer_name_, *this).hideDialog();
}


bool InsetCommand::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (updateButtonLabel_) {
		updateButtonLabel_ = false;
		button_.update(getScreenLabel(mi.base.bv->buffer()),
			       editable() != NOT_EDITABLE);
	}
	button_.metrics(mi, dim);
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


bool InsetCommand::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}


void InsetCommand::draw(PainterInfo & pi, int x, int y) const
{
	button_.setRenderState(mouse_hover_);
	button_.draw(pi, x, y);
}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_ = p;
	updateButtonLabel_ = true;
}


int InsetCommand::latex(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::plaintext(Buffer const & buf, odocstream & os,
			    OutputParams const &) const
{
	docstring const str = "[" + buf.B_("LaTeX Command: ") + from_utf8(getCmdName()) + "]";
	os << str;
	return str.size();
}


int InsetCommand::docbook(Buffer const &, odocstream &,
			  OutputParams const &) const
{
	return 0;
}


void InsetCommand::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_REFRESH:
		updateButtonLabel_ = true;
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(p_.getCmdName());
		InsetCommandMailer::string2params(mailer_name_, to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty())
			cur.noUpdate();
		else
			setParams(p);
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE: {
		string const name = to_utf8(cmd.argument());
		InsetCommandMailer(name, *this).updateDialog(&cur.bv());
		break;
	}

	case LFUN_MOUSE_RELEASE: {
		if (!cur.selection())
			edit(cur, true);
		break;
	}

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}

}


bool InsetCommand::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	// suppress these
	case LFUN_ERT_INSERT:
		status.enabled(false);
		return true;
	// we handle these
	case LFUN_INSET_REFRESH:
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		status.enabled(true);
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


void InsetCommand::edit(Cursor & cur, bool)
{
	if (!mailer_name_.empty())
		InsetCommandMailer(mailer_name_, *this).showDialog(&cur.bv());
}


void InsetCommand::replaceContents(std::string const & from, string const & to)
{
	if (getContents() == from)
		setContents(to);
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
	params.clear();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string n;
	lex >> n;
	if (!lex || n != name)
		return print_mailer_error("InsetCommandMailer", in, 1, name);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
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


} // namespace lyx
