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
#include "BufferView.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxlex.h"

#include "support/std_sstream.h"

using std::auto_ptr;
using std::ostream;


void InsetBranch::init()
{
	setInsetName("Branch");
}


InsetBranch::InsetBranch(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp)
{
	params_.branch = label;
	// Hack: stash the list of all allowable branch labels from this
	// buffer into inset's parm list as a "stowaway":
	params_.branchlist = bp.branchlist;
	init();
}


InsetBranch::InsetBranch(InsetBranch const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


InsetBranch::~InsetBranch()
{
	InsetBranchMailer mailer("branch", *this);
	mailer.hideDialog();
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
	string const color = params_.branchlist.getColor(params_.branch);
	if (!color.empty()) {
		setBackgroundColor(lcolor.getFromLyXName(params_.branch));
	} else
		setBackgroundColor(LColor::background);
	setLabelFont(font);
}


bool InsetBranch::showInsetDialog(BufferView * bv) const
{
	InsetBranchMailer("branch", const_cast<InsetBranch &>(*this)).showDialog(bv);
	return true;
}


dispatch_result InsetBranch::localDispatch(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		{
		InsetBranchParams params;
		InsetBranchMailer::string2params(cmd.argument, params);
		params_.branch = params.branch;
		setButtonLabel();
		bv->updateInset(this);
		return DISPATCHED;
		}
	case LFUN_INSET_EDIT:
		if (cmd.button() != mouse_button::button3)
			return InsetCollapsable::localDispatch(cmd);
		
		return UNDISPATCHED;
	case LFUN_INSET_DIALOG_UPDATE:
		InsetBranchMailer("branch", *this).updateDialog(bv);
		return DISPATCHED;
	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
		    InsetBranchMailer("branch", *this).showDialog(bv);
			return DISPATCHED;
		}
		// fallthrough:
	default:
		return InsetCollapsable::localDispatch(cmd);
	}
}


int InsetBranch::latex(Buffer const & buf, ostream & os,
	LatexRunParams const & runparams) const
{
	string const branch_sel = buf.params.branchlist.allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos)
		return inset.latex(buf, os, runparams);
	return 0;
}


int InsetBranch::linuxdoc(Buffer const &, std::ostream &) const
{
	return 0;
}


int InsetBranch::docbook(Buffer const & buf, std::ostream & os, bool mixcont) const
{
	// untested - MV
	string const branch_sel = buf.params.branchlist.allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos)
		return inset.docbook(buf, os, mixcont);
	return 0;
}


int InsetBranch::ascii(Buffer const & buf, std::ostream & os, int ll) const
{
	string const branch_sel = buf.params.branchlist.allSelected();
	if (branch_sel.find(params_.branch, 0) != string::npos) {
		return inset.ascii(buf, os, ll);
	}
	return 0;
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}



InsetBranchMailer::InsetBranchMailer(string const & name,
						InsetBranch & inset)
	: name_(name), inset_(inset)
{
}


string const InsetBranchMailer::inset2string(Buffer const & buf) const
{
	InsetBranchParams params = inset_.params();
	params.branchlist = buf.params.branchlist;
	inset_.setParams(params);
	return params2string(name_, params);
}


string const InsetBranchMailer::params2string(string const & name,
				InsetBranchParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);
	// Add all_branches parameter to data:
	data << params.branchlist.allBranches() << "\n";
	return STRCONV(data.str());
}


void InsetBranchMailer::string2params(string const & in,
				     InsetBranchParams & params)
{
	params = InsetBranchParams();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);
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
