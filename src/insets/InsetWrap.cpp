/**
 * \file InsetWrap.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetWrap.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Counters.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "OutputParams.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/convert.h"
#include "support/docstream.h"
#include "support/debug.h"
#include "support/gettext.h"

#include "frontends/Application.h"

using namespace std;

namespace lyx {


InsetWrap::InsetWrap(Buffer const & buf, string const & type)
	: InsetCollapsable(buf), name_(from_utf8(type))
{
	setLabel(_("wrap: ") + floatName(type, buf.params()));
	params_.type = type;
	params_.lines = 0;
	params_.placement = "o";
	params_.overhang = Length(0, Length::PCW);
	params_.width = Length(50, Length::PCW);
}


InsetWrap::~InsetWrap()
{
	hideDialogs("ert", this);
}


void InsetWrap::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetWrapParams params;
		InsetWrap::string2params(to_utf8(cmd.argument()), params);
		params_.lines = params.lines;
		params_.placement = params.placement;
		params_.overhang = params.overhang;
		params_.width = params.width;
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("wrap", params2string(params()));
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetWrap::getStatus(Cursor & cur, FuncRequest const & cmd,
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


void InsetWrap::updateLabels(ParIterator const & it)
{
	setLabel(_("wrap: ") + floatName(params_.type, buffer().params()));
	Counters & cnts = buffer().params().documentClass().counters();
	string const saveflt = cnts.current_float();

	// Tell to captions what the current float is
	cnts.current_float(params().type);

	InsetCollapsable::updateLabels(it);

	// reset afterwards
	cnts.current_float(saveflt);
}


void InsetWrapParams::write(ostream & os) const
{
	os << "Wrap " << type << '\n';
	os << "lines " << lines << "\n";
	os << "placement " << placement << "\n";
	os << "overhang " << overhang.asString() << "\n";
	os << "width \"" << width.asString() << "\"\n";
}


void InsetWrapParams::read(Lexer & lex)
{
	string token;

	lex >> token;
	if (token == "lines")
		lex >> lines;
	else {
		lyxerr << "InsetWrap::Read:: Missing 'lines'-tag!"
			<< endl;
		// take countermeasures
		lex.pushToken(token);
	}
	if (!lex)
		return;
	lex >> token;
	if (token == "placement")
		lex >> placement;
	else {
		lyxerr << "InsetWrap::Read:: Missing 'placement'-tag!"
			<< endl;
		lex.pushToken(token);
	}
	if (!lex)
		return;
	lex >> token;
	if (token == "overhang") {
		lex.next();
		overhang = Length(lex.getString());
	} else {
		lyxerr << "InsetWrap::Read:: Missing 'overhang'-tag!"
			<< endl;
		lex.pushToken(token);
	}
	if (!lex)
		return;
	lex >> token;
	if (token == "width") {
		lex.next();
		width = Length(lex.getString());
	} else {
		lyxerr << "InsetWrap::Read:: Missing 'width'-tag!"
			<< endl;
		lex.pushToken(token);
	}
}


void InsetWrap::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetWrap::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


void InsetWrap::validate(LaTeXFeatures & features) const
{
	features.require("wrapfig");
	InsetCollapsable::validate(features);
}


docstring InsetWrap::editMessage() const
{
	return _("Opened Wrap Inset");
}


int InsetWrap::latex(odocstream & os, OutputParams const & runparams) const
{
	os << "\\begin{wrap" << from_ascii(params_.type) << '}';
	// no optional argument when lines are zero
	if (params_.lines != 0)
		os << '[' << params_.lines << ']';
	os << '{' << from_ascii(params_.placement) << '}';
	Length over(params_.overhang);
	// no optional argument when the value is zero
	if (over.value() != 0)
		os << '[' << from_ascii(params_.overhang.asLatexString()) << ']';
	os << '{' << from_ascii(params_.width.asLatexString()) << "}%\n";
	int const i = InsetText::latex(os, runparams);
	os << "\\end{wrap" << from_ascii(params_.type) << "}%\n";
	return i + 2;
}


int InsetWrap::plaintext(odocstream & os, OutputParams const & runparams) const
{
	os << '[' << buffer().B_("wrap") << ' '
		<< floatName(params_.type, buffer().params()) << ":\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetWrap::docbook(odocstream & os, OutputParams const & runparams) const
{
	// FIXME UNICODE
	os << '<' << from_ascii(params_.type) << '>';
	int const i = InsetText::docbook(os, runparams);
	os << "</" << from_ascii(params_.type) << '>';
	return i;
}


bool InsetWrap::insetAllowed(InsetCode code) const
{
	switch(code) {
	case FLOAT_CODE:
	case FOOT_CODE:
	case MARGIN_CODE:
		return false;
	default:
		return InsetCollapsable::insetAllowed(code);
	}
}


bool InsetWrap::showInsetDialog(BufferView * bv) const
{
	if (!InsetText::showInsetDialog(bv))
		bv->showDialog("wrap", params2string(params()),
			const_cast<InsetWrap *>(this));
	return true;
}


void InsetWrap::string2params(string const & in, InsetWrapParams & params)
{
	params = InsetWrapParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != "wrap") {
		LYXERR0("Expected arg 1 to be \"wrap\" in " << in);
		return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "Wrap") {
		LYXERR0("Expected arg 2 to be \"Wrap\" in " << in);
		return;
	}

	// We have to read the type here!
	lex >> params.type;
	params.read(lex);
}


string InsetWrap::params2string(InsetWrapParams const & params)
{
	ostringstream data;
	data << "wrap" << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
