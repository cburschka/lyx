/**
 * \file insetnote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Lsstream.h"

#include "insetnote.h"
#include "metricsinfo.h"
#include "gettext.h"
#include "lyxfont.h"
#include "language.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"

using std::ostream;
using std::auto_ptr;


void InsetNote::init()
{
	setInsetName("Note");
	setButtonLabel();
}


InsetNote::InsetNote(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp)
{
	params_.type = label;
	init();
	setButtonLabel();
}


InsetNote::InsetNote(InsetNote const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


InsetNote::~InsetNote() // MV
{
	InsetNoteMailer mailer("note", *this);
	mailer.hideDialog();
}


auto_ptr<InsetBase> InsetNote::clone() const
{
	return auto_ptr<InsetBase>(new InsetNote(*this));
}


string const InsetNote::editMessage() const
{
	return _("Opened Note Inset");
}


void InsetNote::write(Buffer const * buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetNote::read(Buffer const * buf, LyXLex & lex)
{
	InsetCollapsable::read(buf, lex);
	setButtonLabel();
}


void InsetNote::setButtonLabel()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();

	if (params_.type == "Note") {
		setLabel(_("LyX Note"));
		font.setColor(LColor::note);
		setBackgroundColor(LColor::notebg);
	} else if (params_.type == "Comment") {
		setLabel(_("Comment"));
		font.setColor(LColor::comment);
		setBackgroundColor(LColor::commentbg);
	} else {
		setLabel(_("Greyed Out"));
		font.setColor(LColor::greyedout);
		setBackgroundColor(LColor::greyedoutbg);
	}
	setLabelFont(font);
}


void InsetNote::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	// Contrary to Greyedout, these cannot be construed as part of the
	// running text: make them stand on their own
	if (params_.type == "Note" || params_.type == "Comment")
		if (!collapsed_)
			dim.wid = mi.base.textwidth;
	dim_ = dim;
}


bool InsetNote::showInsetDialog(BufferView * bv) const
{
	InsetNoteMailer("note", const_cast<InsetNote &>(*this)).showDialog(bv);
	return true;
}


dispatch_result InsetNote::localDispatch(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();

	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetNoteParams params;
		InsetNoteMailer::string2params(cmd.argument, params);
		params_.type = params.type;
		setButtonLabel();
		bv->updateInset(this);
		return DISPATCHED;
	}

	case LFUN_INSET_EDIT:
		if (cmd.button() == mouse_button::button3)
			return UNDISPATCHED;
		return InsetCollapsable::localDispatch(cmd);

	case LFUN_INSET_DIALOG_UPDATE:
		InsetNoteMailer("note", *this).updateDialog(bv);
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetNoteMailer("note", *this).showDialog(bv);
			return DISPATCHED;
		}
		// fallthrough:

	default:
		return InsetCollapsable::localDispatch(cmd);
	}
}


int InsetNote::latex(Buffer const * buf, ostream & os,
				                LatexRunParams const & runparams) const
{
	string const pt = params_.type;

	int i = 0;
	if (pt == "Comment")
		 // verbatim
		os << "%\n\\begin{comment}\n";
	else if (pt == "Greyedout")
		 // we roll our own macro
		os << "%\n\\begin{lyxgreyedout}\n";

	if (pt != "Note")
		i = inset.latex(buf, os, runparams);

	if (pt == "Comment") {
		os << "%\n\\end{comment}\n";
		i += 4;
	} else if (pt == "Greyedout") {
		os << "%\n\\end{lyxgreyedout}\n";
		i += 4;
	} 
	return i;
}


int InsetNote::linuxdoc(Buffer const * buf, std::ostream & os) const
{
	string const pt = params_.type;

	int i = 0;
	if (pt == "Comment")
		os << "<comment>\n";

	if (pt != "Note")
		i = inset.linuxdoc(buf, os);

	if (pt == "Comment") {
		os << "\n</comment>\n";
		i += 3;
	}
	return i;
}


int InsetNote::docbook(Buffer const * buf, std::ostream & os, bool mixcont) const
{
	string const pt = params_.type;

	int i = 0;
	if (pt == "Comment")
		os << "<remark>\n";

	if (pt != "Note")
		i = inset.docbook(buf, os, mixcont);

	if (pt == "Comment") {
		os << "\n</remark>\n";
		i += 3;
	}
	return i;
}


int InsetNote::ascii(Buffer const * buf, std::ostream & os, int ll) const
{
	int i = 0;
	string const pt = params_.type;
	if (pt != "Note") {
		os << "[";
		i = inset.ascii(buf, os, ll);
		os << "]";
	}
	return i;
}


void InsetNote::validate(LaTeXFeatures & features) const
{
	if (params_.type == "Comment")
		features.require("verbatim");
	if (params_.type == "Greyedout") {
		features.require("color");
		features.require("lyxgreyedout");
	}
	inset.validate(features);
}



InsetNoteMailer::InsetNoteMailer(string const & name,
						InsetNote & inset)
	: name_(name), inset_(inset)
{
}


string const InsetNoteMailer::inset2string(Buffer const &) const
{
	return params2string(name_, inset_.params());
}


string const InsetNoteMailer::params2string(string const & name,
				InsetNoteParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);
	return STRCONV(data.str());
}


void InsetNoteMailer::string2params(string const & in,
				     InsetNoteParams & params)
{
	params = InsetNoteParams();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);
	params.read(lex);
}


void InsetNoteParams::write(ostream & os) const
{
	os << type << "\n";
}


void InsetNoteParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
	}

	if (lex.isOK()) {
		lex.next();
		type = lex.getString();
	}
}
