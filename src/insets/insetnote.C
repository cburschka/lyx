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

#include "insetnote.h"

#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "support/lyxalgo.h"
#include "support/std_sstream.h"
#include "support/translator.h"


using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

typedef Translator<std::string, InsetNoteParams::Type> NoteTranslator;

NoteTranslator const init_notetranslator() {
	NoteTranslator translator("Note", InsetNoteParams::Note);
	translator.addPair("Comment", InsetNoteParams::Comment);
	translator.addPair("Greyedout", InsetNoteParams::Greyedout);
	return translator;
}


NoteTranslator const init_notetranslator_loc() {
	NoteTranslator translator(_("Note"), InsetNoteParams::Note);
	translator.addPair(_("Comment"), InsetNoteParams::Comment);
	translator.addPair(_("Greyed out"), InsetNoteParams::Greyedout);
	return translator;
}


NoteTranslator const & notetranslator() {
	static NoteTranslator translator = init_notetranslator();
	return translator;
}


NoteTranslator const & notetranslator_loc() {
	static NoteTranslator translator = init_notetranslator_loc();
	return translator;
}

} // anon




InsetNoteParams::InsetNoteParams()
	: type(Note)
{}


void InsetNoteParams::write(ostream & os) const
{
	string const label = notetranslator().find(type);
	os << "Note " << label << "\n";
}


void InsetNoteParams::read(LyXLex & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = notetranslator().find(label);
}


void InsetNote::init()
{
	setInsetName("Note");
	setButtonLabel();
}


InsetNote::InsetNote(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp)
{
	params_.type = notetranslator().find(label);
	init();
}


InsetNote::InsetNote(InsetNote const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


InsetNote::~InsetNote()
{
	InsetNoteMailer(*this).hideDialog();
}


auto_ptr<InsetBase> InsetNote::clone() const
{
	return auto_ptr<InsetBase>(new InsetNote(*this));
}


string const InsetNote::editMessage() const
{
	return _("Opened Note Inset");
}


void InsetNote::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetNote::read(Buffer const & buf, LyXLex & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
	setButtonLabel();
}


void InsetNote::setButtonLabel()
{
	string const label = notetranslator_loc().find(params_.type);
	setLabel(label);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();

	switch (params_.type) {
	case InsetNoteParams::Note:
		font.setColor(LColor::note);
		setBackgroundColor(LColor::notebg);
		break;
	case InsetNoteParams::Comment:
		font.setColor(LColor::comment);
		setBackgroundColor(LColor::commentbg);
		break;
	case InsetNoteParams::Greyedout:
		font.setColor(LColor::greyedout);
		setBackgroundColor(LColor::greyedoutbg);
		break;
	}
	setLabelFont(font);
}


bool InsetNote::showInsetDialog(BufferView * bv) const
{
	InsetNoteMailer(const_cast<InsetNote &>(*this)).showDialog(bv);
	return true;
}


void InsetNote::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		InsetNoteMailer::string2params(cmd.argument, params_);
		setButtonLabel();
		cur.bv().update();
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		InsetNoteMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd))
			InsetNoteMailer(*this).showDialog(&cur.bv());
		else
			InsetCollapsable::priv_dispatch(cur, cmd);
		break;

	default:
		InsetCollapsable::priv_dispatch(cur, cmd);
		break;
	}
}


int InsetNote::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	string type;
	if (params_.type == InsetNoteParams::Comment)
		type = "comment";
	else if (params_.type == InsetNoteParams::Greyedout)
		type = "lyxgreyedout";

	ostringstream ss;
	ss << "%\n\\begin{" << type << "}\n";
	InsetText::latex(buf, ss, runparams);
	ss << "%\n\\end{" << type << "}\n";

	string const str = ss.str();
	os << str;
	// Return how many newlines we issued.
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetNote::linuxdoc(Buffer const & buf, std::ostream & os,
			OutputParams const & runparams) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	ostringstream ss;
	if (params_.type == InsetNoteParams::Comment)
		ss << "<comment>\n";

	InsetText::linuxdoc(buf, ss, runparams);

	if (params_.type == InsetNoteParams::Comment)
		ss << "\n</comment>\n";

	string const str = ss.str();
	os << str;
	// Return how many newlines we issued.
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetNote::docbook(Buffer const & buf, std::ostream & os,
		       OutputParams const & runparams) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	ostringstream ss;
	if (params_.type == InsetNoteParams::Comment)
		ss << "<remark>\n";

	InsetText::docbook(buf, ss, runparams);

	if (params_.type == InsetNoteParams::Comment)
		ss << "\n</remark>\n";

	string const str = ss.str();
	os << str;
	// Return how many newlines we issued.
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetNote::plaintext(Buffer const & buf, std::ostream & os,
		     OutputParams const & runparams) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	ostringstream ss;
	ss << "[";
	InsetText::plaintext(buf, ss, runparams);
	ss << "]";

	string const str = ss.str();
	os << str;
	// Return how many newlines we issued.
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


void InsetNote::validate(LaTeXFeatures & features) const
{
	if (params_.type == InsetNoteParams::Comment)
		features.require("verbatim");
	if (params_.type == InsetNoteParams::Greyedout) {
		features.require("color");
		features.require("lyxgreyedout");
	}
	InsetText::validate(features);
}



string const InsetNoteMailer::name_("note");

InsetNoteMailer::InsetNoteMailer(InsetNote & inset)
	: inset_(inset)
{}


string const InsetNoteMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


string const InsetNoteMailer::params2string(InsetNoteParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}


void InsetNoteMailer::string2params(string const & in,
				    InsetNoteParams & params)
{
	params = InsetNoteParams();

	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetNoteMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by LyXText::readInset
	string id;
	lex >> id;
	if (!lex || id != "Note")
		return print_mailer_error("InsetBoxMailer", in, 2, "Note");

	params.read(lex);
}
