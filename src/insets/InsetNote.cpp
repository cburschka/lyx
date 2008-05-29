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
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/Translator.h"

#include "frontends/Application.h"

#include <algorithm>
#include <sstream>

using namespace std;

namespace lyx {

namespace {

typedef Translator<string, InsetNoteParams::Type> NoteTranslator;
typedef Translator<docstring, InsetNoteParams::Type> NoteTranslatorLoc;

NoteTranslator const init_notetranslator()
{
	NoteTranslator translator("Note", InsetNoteParams::Note);
	translator.addPair("Comment", InsetNoteParams::Comment);
	translator.addPair("Greyedout", InsetNoteParams::Greyedout);
	return translator;
}


NoteTranslatorLoc const init_notetranslator_loc()
{
	NoteTranslatorLoc translator(_("Note[[InsetNote]]"), InsetNoteParams::Note);
	translator.addPair(_("Comment"), InsetNoteParams::Comment);
	translator.addPair(_("Greyed out"), InsetNoteParams::Greyedout);
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


/////////////////////////////////////////////////////////////////////
//
// InsetNode
//
/////////////////////////////////////////////////////////////////////

InsetNote::InsetNote(Buffer const & buf, string const & label)
	: InsetCollapsable(buf)
{
	params_.type = notetranslator().find(label);
}


InsetNote::~InsetNote()
{
	hideDialogs("note", this);
}


docstring InsetNote::editMessage() const
{
	return _("Opened Note Inset");
}


docstring InsetNote::name() const 
{
	return from_ascii("Note:" + notetranslator().find(params_.type));
}


Inset::DisplayType InsetNote::display() const
{
	return Inline;
}


void InsetNote::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetNote::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


void InsetNote::setButtonLabel()
{
	docstring const label = notetranslator_loc().find(params_.type);
	setLabel(label);
}


bool InsetNote::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("note", params2string(params()),
		const_cast<InsetNote *>(this));
	return true;
}


void InsetNote::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		string2params(to_utf8(cmd.argument()), params_);
		// get a bp from cur:
		setLayout(cur.buffer().params());
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("note", params2string(params()));
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
		// disallow comment and greyed out in commands
		flag.setEnabled(!cur.paragraph().layout().isCommand() ||
				cmd.getArg(2) == "Note");
		if (cmd.getArg(0) == "note") {
			InsetNoteParams params;
			string2params(to_utf8(cmd.argument()), params);
			flag.setOnOff(params_.type == params.type);
		}
		return true;

	case LFUN_INSET_DIALOG_UPDATE:
		flag.setEnabled(true);
		return true;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}


void InsetNote::addToToc(DocIterator const & cpit)
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(*this));

	Toc & toc = buffer().tocBackend().toc("note");
	docstring str;
	str = notetranslator_loc().find(params_.type) + from_ascii(": ")
		+ getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
}


bool InsetNote::isMacroScope() const
{
	// LyX note has no latex output
	if (params_.type == InsetNoteParams::Note)
		return true;

	return InsetCollapsable::isMacroScope();
}


int InsetNote::latex(odocstream & os, OutputParams const & runparams_in) const
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
	InsetCollapsable::latex(ss, runparams);
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
	return int(count(str.begin(), str.end(), '\n'));
}


int InsetNote::plaintext(odocstream & os,
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
	os << '[' << buffer().B_("note") << ":\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetNote::docbook(odocstream & os, OutputParams const & runparams_in) const
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

	int const n = InsetText::docbook(os, runparams);

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
	InsetText::validate(features);
}


docstring InsetNote::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-note");
}


string InsetNote::params2string(InsetNoteParams const & params)
{
	ostringstream data;
	data << "note" << ' ';
	params.write(data);
	return data.str();
}


void InsetNote::string2params(string const & in, InsetNoteParams & params)
{
	params = InsetNoteParams();

	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetNote::string2params");
	lex >> "note" >> "Note";

	params.read(lex);
}


} // namespace lyx
