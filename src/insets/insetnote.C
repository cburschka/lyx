/**
 * \file insetnote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "Lsstream.h"

#include "insetnote.h"
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
	setLabel(label);
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


InsetBase * InsetNote::clone() const
{
	return new InsetNote(*this);
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

	setLabel(params_.type);
	if (params_.type == "Note" || params_.type == "Comment") {
		font.setColor(LColor::note);
		setBackgroundColor(LColor::notebg);
	} else {
		font.setColor(LColor::red);
		setBackgroundColor(LColor::background);
	}
	setLabelFont(font);
}


dispatch_result InsetNote::localDispatch(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();

	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		{
		InsetNoteParams params;
		InsetNoteMailer::string2params(cmd.argument, params);
		params_.type = params.type;
		setButtonLabel();
		bv->updateInset(this);
		return DISPATCHED;
		}
	case LFUN_INSET_EDIT:
		if (cmd.button() != mouse_button::button3) 
			return InsetCollapsable::localDispatch(cmd);
		return UNDISPATCHED;
	case LFUN_INSET_DIALOG_UPDATE:
		InsetNoteMailer("note", *this).updateDialog(bv);
		return DISPATCHED;
	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && cmd.x < button_length 
					&& cmd.y >= button_top_y && cmd.y <= button_bottom_y) {
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
	if (pt == "Comment") os << "%\n\\begin{comment}\n"; // remember to validate
	if (pt == "Greyedout") os << "%\n\\textcolor[gray]{0.8}{";
	if (pt != "Note") {
		i = inset.latex(buf, os, runparams);
	}
	if (pt == "Comment") {
		os << "%\n\\end{comment}\n";
 		i += 3;
	}
	if (pt == "Greyedout") { 
		os << "%\n}";
		i += 2;
	}
	return i;
}


int InsetNote::linuxdoc(Buffer const *, std::ostream &) const
{ 
	return 0; 
}


int InsetNote::docbook(Buffer const * buf, std::ostream & os, bool mixcont) const
{
	int i = 0;
	string const pt = params_.type;
	// incomplete, untested - MV
	if (pt != "Note") 
		i = inset.docbook(buf, os, mixcont);
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
	if (params_.type == "Greyedout")
		features.require("color");
	inset.validate(features);
}



InsetNoteMailer::InsetNoteMailer(string const & name,
						InsetNote & inset)
	: name_(name), inset_(inset)
{
}


string const InsetNoteMailer::inset2string() const
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

