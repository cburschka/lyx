/**
 * \file insetbranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbranch.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BranchList.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxlex.h"
#include "paragraph.h"

#include "support/std_sstream.h"

using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


void InsetBranch::init()
{
	setInsetName("Branch");
	setButtonLabel();
}


InsetBranch::InsetBranch(BufferParams const & bp,
			 InsetBranchParams const & params)
	: InsetCollapsable(bp), params_(params)
{
	init();
}


InsetBranch::InsetBranch(InsetBranch const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


InsetBranch::~InsetBranch()
{
	InsetBranchMailer(*this).hideDialog();
}


auto_ptr<InsetBase> InsetBranch::clone() const
{
	return auto_ptr<InsetBase>(new InsetBranch(*this));
}


string const InsetBranch::editMessage() const
{
	return _("Opened Branch Inset");
}


void InsetBranch::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetBranch::read(Buffer const & buf, LyXLex & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
	setButtonLabel();
}


void InsetBranch::setButtonLabel()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();

	setLabel("Branch: " + params_.branch);
	font.setColor(LColor::foreground);
	if (!params_.branch.empty())
		setBackgroundColor(lcolor.getFromLyXName(params_.branch));
	else
		setBackgroundColor(LColor::background);
	setLabelFont(font);
}


bool InsetBranch::showInsetDialog(BufferView * bv) const
{
	InsetBranchMailer(const_cast<InsetBranch &>(*this)).showDialog(bv);
	return true;
}


DispatchResult
InsetBranch::priv_dispatch(FuncRequest const & cmd,
			   idx_type & idx, pos_type & pos)
{
	BufferView * bv = cmd.view();
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetBranchParams params;
		InsetBranchMailer::string2params(cmd.argument, params);
		params_.branch = params.branch;
		setButtonLabel();
		return DispatchResult(true, true);
	}

	case LFUN_MOUSE_PRESS:
		if (cmd.button() != mouse_button::button3)
			return InsetCollapsable::priv_dispatch(cmd, idx, pos);
		return DispatchResult(false);

	case LFUN_INSET_DIALOG_UPDATE:
		InsetBranchMailer(*this).updateDialog(bv);
		return DispatchResult(true);

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetBranchMailer(*this).showDialog(bv);
			return DispatchResult(true);
		}
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);

	default:
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


bool InsetBranch::isBranchSelected(BranchList const & branchlist) const
{
	BranchList::const_iterator const end = branchlist.end();
	BranchList::const_iterator it =
		std::find_if(branchlist.begin(), end,
			     BranchNamesEqual(params_.branch));
	if (it == end)
		return false;
	return it->getSelected();
}


int InsetBranch::latex(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	return isBranchSelected(buf.params().branchlist()) ?
		inset.latex(buf, os, runparams) : 0;
}


int InsetBranch::linuxdoc(Buffer const & buf, std::ostream & os,
			  OutputParams const & runparams) const
{
	return isBranchSelected(buf.params().branchlist()) ?
		inset.linuxdoc(buf, os, runparams) : 0;
}


int InsetBranch::docbook(Buffer const & buf, std::ostream & os,
			 OutputParams const & runparams) const
{
	return isBranchSelected(buf.params().branchlist()) ?
		inset.docbook(buf, os, runparams) : 0;
}


int InsetBranch::plaintext(Buffer const & buf, std::ostream & os,
			   OutputParams const & runparams) const
{
	return isBranchSelected(buf.params().branchlist()) ?
		inset.plaintext(buf, os, runparams): 0;
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}



string const InsetBranchMailer:: name_("branch");

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
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (name != name_)
		return print_mailer_error("InsetBranchMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by LyXText::readInset
	string id;
	lex >> id;
	if (!lex || id != "Branch")
		return print_mailer_error("InsetBranchMailer", in, 2, "Branch");

	params.read(lex);
}


void InsetBranchParams::write(ostream & os) const
{
	os << "Branch " << branch << '\n';
}


void InsetBranchParams::read(LyXLex & lex)
{
	lex >> branch;
}
