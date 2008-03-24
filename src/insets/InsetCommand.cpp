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
#include "support/gettext.h"
#include "Lexer.h"
#include "MetricsInfo.h"

#include <sstream>

using namespace std;

namespace lyx {


// FIXME Would it now be possible to use the InsetCode in 
// place of the mailer name and recover that information?
InsetCommand::InsetCommand(InsetCommandParams const & p,
			   string const & mailer_name)
	: p_(p),
	  mailer_name_(mailer_name),
	  mouse_hover_(false)
{}


InsetCommand::~InsetCommand()
{
	if (!mailer_name_.empty())
		InsetCommandMailer(mailer_name_, *this).hideDialog();
}


void InsetCommand::metrics(MetricsInfo & mi, Dimension & dim) const
{
	button_.update(screenLabel(), editable() != NOT_EDITABLE);
	button_.metrics(mi, dim);
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
	initView();
}


int InsetCommand::latex(odocstream & os, OutputParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::plaintext(odocstream & os, OutputParams const &) const
{
	docstring const str = "[" + buffer().B_("LaTeX Command: ")
		+ from_utf8(getCmdName()) + "]";
	os << str;
	return str.size();
}


int InsetCommand::docbook(odocstream &, OutputParams const &) const
{
	return 0;
}


void InsetCommand::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) == "changetype") {
			p_.setCmdName(cmd.getArg(1));
			initView();
			break;
		}
		InsetCommandParams p(p_.code());
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
		if (!cur.selection() && cmd.button() != mouse_button::button3)
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
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			string const newtype = cmd.getArg(1);
			status.enabled(p_.isCompatibleCommand(p_.code(), newtype));
			status.setOnOff(newtype == p_.getCmdName());
		} else
			status.enabled(true);
		return true;
	case LFUN_INSET_DIALOG_UPDATE:
		status.enabled(true);
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


docstring InsetCommand::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-") + from_ascii(mailer_name_);
}


void InsetCommand::edit(Cursor & cur, bool, EntryDirection)
{
	if (!mailer_name_.empty())
		InsetCommandMailer(mailer_name_, *this).showDialog(&cur.bv());
}


InsetCommandMailer::InsetCommandMailer(
	string const & name, InsetCommand & inset)
	: name_(name), inset_(inset)
{}


string const InsetCommandMailer::inset2string(Buffer const &) const
{
	return params2string(name(), inset_.params());
}


//FIXME This could take an InsetCode instead of a string
bool InsetCommandMailer::string2params(
	string const & name, string const & in, InsetCommandParams & params)
{
	params.clear();
	if (in.empty())
		return false;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string n;
	lex >> n;
	if (!lex || n != name) {
		print_mailer_error("InsetCommandMailer", in, 1, name);
		return false;
	}

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "CommandInset") {
		print_mailer_error("InsetCommandMailer", in, 2, "LatexCommand");
		return false;
	}

	params.read(lex);
	return true;
}


//FIXME This could take an InsetCode instead of a string
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
