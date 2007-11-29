/**
 * \file InsetBranch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBranch.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"
#include "Color.h"
#include "Counters.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
#include "Lexer.h"
#include "OutputParams.h"
#include "TextClass.h"

#include <sstream>


namespace lyx {

using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;


InsetBranch::InsetBranch(BufferParams const & bp,
			 InsetBranchParams const & params)
	: InsetCollapsable(bp), params_(params)
{}


InsetBranch::InsetBranch(InsetBranch const & in)
	: InsetCollapsable(in), params_(in.params_)
{}


InsetBranch::~InsetBranch()
{
	InsetBranchMailer(*this).hideDialog();
}


Inset * InsetBranch::clone() const
{
	return new InsetBranch(*this);
}


docstring const InsetBranch::editMessage() const
{
	return _("Opened Branch Inset");
}


void InsetBranch::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetBranch::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


void InsetBranch::setButtonLabel()
{
	docstring s = _("Branch: ") + params_.branch;
	if (!params_.branch.empty()) {
		// FIXME UNICODE
		ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
		if (c == Color_none) {
			s = _("Undef: ") + s;
		}
	}
	if (decoration() == Classic)
		setLabel(isOpen() ? s : getNewLabel(s) );
	else
		setLabel(params_.branch + ": " + getNewLabel(s));
}


ColorCode InsetBranch::backgroundColor() const
{
	if (!params_.branch.empty()) {
		// FIXME UNICODE
		ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
		if (c == Color_none) {
			c = Color_error;
		}
		return c;
	} else
		return Inset::backgroundColor();
}


bool InsetBranch::showInsetDialog(BufferView * bv) const
{
	InsetBranchMailer(const_cast<InsetBranch &>(*this)).showDialog(bv);
	return true;
}


void InsetBranch::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetBranchParams params;
		InsetBranchMailer::string2params(to_utf8(cmd.argument()), params);
		params_.branch = params.branch;
		setLayout(cur.buffer().params());
		break;
	}

	case LFUN_MOUSE_PRESS:
		if (cmd.button() != mouse_button::button3)
			InsetCollapsable::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		InsetBranchMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd))
			InsetBranchMailer(*this).showDialog(&cur.bv());
		else
			InsetCollapsable::doDispatch(cur, cmd);
		break;


	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "assign") {
			// The branch inset uses "assign".
			if (isBranchSelected(cur.buffer())) {
				if (status() != Open)
					setStatus(cur, Open);
				else
					cur.undispatched();
			} else {
				if (status() != Collapsed)
					setStatus(cur, Collapsed);
				else
					cur.undispatched();
			}
		}
		else
			InsetCollapsable::doDispatch(cur, cmd);
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetBranch::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open" || cmd.argument() == "close" ||
		    cmd.argument() == "toggle")
			flag.enabled(true);
		else if (cmd.argument() == "assign"
			   || cmd.argument().empty()) {
			if (isBranchSelected(cur.buffer()))
				flag.enabled(status() != Open);
			else
				flag.enabled(status() != Collapsed);
		} else
			flag.enabled(true);
		break;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
	return true;
}


bool InsetBranch::isBranchSelected(Buffer const & buffer) const
{
	Buffer const & realbuffer = *buffer.masterBuffer();
	BranchList const & branchlist = realbuffer.params().branchlist();
	BranchList::const_iterator const end = branchlist.end();
	BranchList::const_iterator it =
		std::find_if(branchlist.begin(), end,
			     BranchNamesEqual(params_.branch));
	if (it == end)
		return false;
	return it->getSelected();
}


void InsetBranch::updateLabels(Buffer const & buf, ParIterator const & it)
{
	if (isBranchSelected(buf))
		InsetCollapsable::updateLabels(buf, it);
	else {
		TextClass const & tclass = buf.params().getTextClass();
		Counters savecnt = tclass.counters();
		InsetCollapsable::updateLabels(buf, it);
		tclass.counters() = savecnt;
	}
}


int InsetBranch::latex(Buffer const & buf, odocstream & os,
		       OutputParams const & runparams) const
{
	return isBranchSelected(buf) ?
		InsetText::latex(buf, os, runparams) : 0;
}


int InsetBranch::plaintext(Buffer const & buf, odocstream & os,
			   OutputParams const & runparams) const
{
	if (!isBranchSelected(buf))
		return 0;

	os << '[' << buf.B_("branch") << ' ' << params_.branch << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetBranch::docbook(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
{
	return isBranchSelected(buf) ?
		InsetText::docbook(buf, os, runparams) : 0;
}


void InsetBranch::textString(Buffer const & buf, odocstream & os) const
{
	if (isBranchSelected(buf))
		os << paragraphs().begin()->asString(buf, true);
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	InsetText::validate(features);
}



string const InsetBranchMailer::name_("branch");

InsetBranchMailer::InsetBranchMailer(InsetBranch & inset)
	: inset_(inset)
{}


string const InsetBranchMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


string const InsetBranchMailer::params2string(InsetBranchParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}


void InsetBranchMailer::string2params(string const & in,
				      InsetBranchParams & params)
{
	params = InsetBranchParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (name != name_)
		return print_mailer_error("InsetBranchMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "Branch")
		return print_mailer_error("InsetBranchMailer", in, 2, "Branch");

	params.read(lex);
}


void InsetBranchParams::write(ostream & os) const
{
	os << "Branch " << to_utf8(branch) << '\n';
}


void InsetBranchParams::read(Lexer & lex)
{
	lex >> branch;
}

} // namespace lyx
