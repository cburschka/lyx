/**
 * \file InsetNote.cpp
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

#include "InsetNote.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "support/docstream.h"
#include "support/Translator.h"

#include <algorithm>
#include <sstream>


namespace lyx {

using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

typedef Translator<std::string, InsetNoteParams::Type> NoteTranslator;
typedef Translator<docstring, InsetNoteParams::Type> NoteTranslatorLoc;

NoteTranslator const init_notetranslator()
{
	NoteTranslator translator("Note", InsetNoteParams::Note);
	translator.addPair("Comment", InsetNoteParams::Comment);
	translator.addPair("Greyedout", InsetNoteParams::Greyedout);
	translator.addPair("Framed", InsetNoteParams::Framed);
	translator.addPair("Shaded", InsetNoteParams::Shaded);
	return translator;
}


NoteTranslatorLoc const init_notetranslator_loc()
{
	NoteTranslatorLoc translator(_("Note"), InsetNoteParams::Note);
	translator.addPair(_("Comment"), InsetNoteParams::Comment);
	translator.addPair(_("Greyed out"), InsetNoteParams::Greyedout);
	translator.addPair(_("Framed"), InsetNoteParams::Framed);
	translator.addPair(_("Shaded"), InsetNoteParams::Shaded);
	return translator;
}


NoteTranslator const & notetranslator()
{
	static NoteTranslator translator = init_notetranslator();
	return translator;
}


NoteTranslatorLoc const & notetranslator_loc()
{
	static NoteTranslatorLoc translator = init_notetranslator_loc();
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


void InsetNoteParams::read(Lexer & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = notetranslator().find(label);
}


InsetNote::InsetNote(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp)
{
	params_.type = notetranslator().find(label);
	setLayout(bp);
	setButtonLabel();
}


InsetNote::InsetNote(InsetNote const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	setButtonLabel();
}


InsetNote::~InsetNote()
{
	InsetNoteMailer(*this).hideDialog();
}


Inset * InsetNote::clone() const
{
	return new InsetNote(*this);
}


docstring const InsetNote::editMessage() const
{
	return _("Opened Note Inset");
}


docstring InsetNote::name() const 
{
	return from_ascii(string("Note") + string(":") + string(notetranslator().find(params_.type)));
}


Inset::DisplayType InsetNote::display() const
{
	switch (params_.type) {
	case InsetNoteParams::Framed:
	case InsetNoteParams::Shaded:
		return AlignLeft;
	default:
		return Inline;
	}
}


void InsetNote::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetNote::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
	setLayout(buf.params());
	setButtonLabel();
}


void InsetNote::setButtonLabel()
{
	docstring const label = notetranslator_loc().find(params_.type);
	setLabel(label);
}


bool InsetNote::showInsetDialog(BufferView * bv) const
{
	InsetNoteMailer(const_cast<InsetNote &>(*this)).showDialog(bv);
	return true;
}


void InsetNote::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		InsetNoteMailer::string2params(to_utf8(cmd.argument()), params_);
		// get a bp from cur:
		setLayout(cur.buffer().params());
		setButtonLabel();
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		InsetNoteMailer(*this).updateDialog(&cur.bv());
		break;
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetNote::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}

void InsetNote::updateLabels(Buffer const & buf, ParIterator const & it)
{
	TextClass const & tclass = buf.params().getTextClass();
	Counters savecnt = tclass.counters();
	InsetCollapsable::updateLabels(buf, it);
	tclass.counters() = savecnt;
}


int InsetNote::latex(Buffer const & buf, odocstream & os,
		     OutputParams const & runparams_in) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	OutputParams runparams(runparams_in);
	if (params_.type == InsetNoteParams::Comment) {
		runparams.inComment = true;
		// Ignore files that are exported inside a comment
		runparams.exportdata.reset(new ExportData);
	} 

	odocstringstream ss;
	InsetCollapsable::latex(buf, ss, runparams);
	// the space after the comment in 'a[comment] b' will be eaten by the
	// comment environment since the space before b is ignored with the
	// following latex output:
	//
	// a%
	// \begin{comment}
	// comment
	// \end{comment}
	//  b
	//
	// Adding {} before ' b' fixes this.
	if (params_.type == InsetNoteParams::Comment)
		ss << "{}";

	docstring const str = ss.str();
	os << str;
	runparams_in.encoding = runparams.encoding;
	// Return how many newlines we issued.
	return int(std::count(str.begin(), str.end(), '\n'));
}


int InsetNote::plaintext(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams_in) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	OutputParams runparams(runparams_in);
	if (params_.type == InsetNoteParams::Comment) {
		runparams.inComment = true;
		// Ignore files that are exported inside a comment
		runparams.exportdata.reset(new ExportData);
	}
	os << '[' << buf.B_("note") << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetNote::docbook(Buffer const & buf, odocstream & os,
		       OutputParams const & runparams_in) const
{
	if (params_.type == InsetNoteParams::Note)
		return 0;

	OutputParams runparams(runparams_in);
	if (params_.type == InsetNoteParams::Comment) {
		os << "<remark>\n";
		runparams.inComment = true;
		// Ignore files that are exported inside a comment
		runparams.exportdata.reset(new ExportData);
	}

	int const n = InsetText::docbook(buf, os, runparams);

	if (params_.type == InsetNoteParams::Comment)
		os << "\n</remark>\n";

	// Return how many newlines we issued.
	//return int(count(str.begin(), str.end(), '\n'));
	return n + 1 + 2;
}


void InsetNote::validate(LaTeXFeatures & features) const
{
	if (params_.type == InsetNoteParams::Comment)
		features.require("verbatim");
	if (params_.type == InsetNoteParams::Greyedout) {
		features.require("color");
		features.require("lyxgreyedout");
	}
	if (params_.type == InsetNoteParams::Shaded) {
		features.require("color");
		features.require("framed");
	}
	if (params_.type == InsetNoteParams::Framed)
		features.require("framed");
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
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetNoteMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "Note")
		return print_mailer_error("InsetBoxMailer", in, 2, "Note");

	params.read(lex);
}


} // namespace lyx
