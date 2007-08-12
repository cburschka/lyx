/**
 * \file InsetWrap.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
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
#include "debug.h"
#include "DispatchResult.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "Lexer.h"
#include "OutputParams.h"
#include "TocBackend.h"

#include "support/convert.h"


namespace lyx {

using std::string;
using std::endl;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


InsetWrap::InsetWrap(BufferParams const & bp, string const & type)
	: InsetCollapsable(bp), name_(from_utf8(type))
{
	setLabel(_("wrap: ") + floatName(type, bp));
	Font font(Font::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(Color::collapsable);
	setLabelFont(font);
	params_.type = type;
	params_.width = Length(50, Length::PCW);
}


InsetWrap::~InsetWrap()
{
	InsetWrapMailer(*this).hideDialog();
}


void InsetWrap::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetWrapParams params;
		InsetWrapMailer::string2params(to_utf8(cmd.argument()), params);
		params_.placement = params.placement;
		params_.width     = params.width;
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetWrapMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetWrapMailer(*this).showDialog(&cur.bv());
			break;
		}
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}

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


void InsetWrap::updateLabels(Buffer const & buf, ParIterator const & it)
{
	Counters & cnts = buf.params().getTextClass().counters();
	string const saveflt = cnts.current_float();

	// Tell to captions what the current float is
	cnts.current_float(params().type);

	InsetCollapsable::updateLabels(buf, it);

	//reset afterwards
	cnts.current_float(saveflt);
}


void InsetWrapParams::write(ostream & os) const
{
	os << "Wrap " << type << '\n';

	if (!placement.empty())
		os << "placement " << placement << "\n";

	os << "width \"" << width.asString() << "\"\n";
}


void InsetWrapParams::read(Lexer & lex)
{
	string token;
	lex >> token;
	if (token == "placement")
		lex >> placement;
	else {
		// take countermeasures
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
		// take countermeasures
		lex.pushToken(token);
	}
}


void InsetWrap::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetWrap::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


void InsetWrap::validate(LaTeXFeatures & features) const
{
	features.require("floatflt");
	InsetCollapsable::validate(features);
}


auto_ptr<Inset> InsetWrap::doClone() const
{
	return auto_ptr<Inset>(new InsetWrap(*this));
}


docstring const InsetWrap::editMessage() const
{
	return _("Opened Wrap Inset");
}


int InsetWrap::latex(Buffer const & buf, odocstream & os,
		     OutputParams const & runparams) const
{
	os << "\\begin{floating" << from_ascii(params_.type) << '}';
	if (!params_.placement.empty())
		os << '[' << from_ascii(params_.placement) << ']';
	os << '{' << from_ascii(params_.width.asLatexString()) << "}%\n";
	int const i = InsetText::latex(buf, os, runparams);
	os << "\\end{floating" << from_ascii(params_.type) << "}%\n";
	return i + 2;
}


int InsetWrap::plaintext(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
{
	os << '[' << buf.B_("wrap") << ' ' << floatName(params_.type, buf.params()) << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetWrap::docbook(Buffer const & buf, odocstream & os,
		       OutputParams const & runparams) const
{
	// FIXME UNICODE
	os << '<' << from_ascii(params_.type) << '>';
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</" << from_ascii(params_.type) << '>';
	return i;
}


bool InsetWrap::insetAllowed(Inset::Code code) const
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
		InsetWrapMailer(const_cast<InsetWrap &>(*this)).showDialog(bv);
	return true;
}


string const InsetWrapMailer::name_("wrap");

InsetWrapMailer::InsetWrapMailer(InsetWrap & inset)
	: inset_(inset)
{}


string const InsetWrapMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetWrapMailer::string2params(string const & in, InsetWrapParams & params)
{
	params = InsetWrapParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetWrapMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "Wrap")
		return print_mailer_error("InsetBoxMailer", in, 2, "Wrap");

	// We have to read the type here!
	lex >> params.type;
	params.read(lex);
}


string const InsetWrapMailer::params2string(InsetWrapParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
