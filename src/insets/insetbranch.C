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


InsetBranch::InsetBranch(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp)
{
	params_.branch = label;
	// Hack: stash the list of all allowable branch labels from this
	// buffer into inset's parm list as a "stowaway":
	params_.branchlist = bp.branchlist();
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
	if (lex.isOK()) {
		lex.next();
		params_.branch = lex.getString();
	}
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


int InsetBranch::latex(Buffer const & buf, ostream & os,
	OutputParams const & runparams) const
{
	string const branch_sel = buf.params().branchlist().allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos)
		return inset.latex(buf, os, runparams);
	return 0;
}


int InsetBranch::linuxdoc(Buffer const & buf, std::ostream & os,
			  OutputParams const & runparams) const
{
	string const branch_sel = buf.params().branchlist().allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos)
		return inset.linuxdoc(buf, os, runparams);
	return 0;
}


int InsetBranch::docbook(Buffer const & buf, std::ostream & os,
			 OutputParams const & runparams) const
{
	string const branch_sel = buf.params().branchlist().allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos)
		return inset.docbook(buf, os, runparams);
	return 0;
}


int InsetBranch::plaintext(Buffer const & buf, std::ostream & os,
		       OutputParams const & runparams) const
{
	string const branch_sel = buf.params().branchlist().allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos) {
		return inset.plaintext(buf, os, runparams);
	}
	return 0;
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}



string const InsetBranchMailer:: name_("branch");

InsetBranchMailer::InsetBranchMailer(InsetBranch & inset)
	: inset_(inset)
{}


string const InsetBranchMailer::inset2string(Buffer const & buf) const
{
	InsetBranchParams params = inset_.params();
	params.branchlist = buf.params().branchlist();
	inset_.setParams(params);
	return params2string(params);
}


string const InsetBranchMailer::params2string(InsetBranchParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	// Add all_branches parameter to data:
	data << params.branchlist.allBranches() << "\n";
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

	params.read(lex);
	// Process all_branches here:
	if (lex.isOK()) {
		lex.next();
		params.branchlist.add(lex.getString());
	}
}


void InsetBranchParams::write(ostream & os) const
{
	os << "Branch" << " " << branch << "\n";
}


void InsetBranchParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
	}
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
	}
	if (lex.isOK()) {
		lex.next();
		branch = lex.getString();
	}
}
