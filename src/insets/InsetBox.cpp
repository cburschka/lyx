/**
 * \file InsetBox.cpp
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

#include "InsetBox.h"

#include "BufferView.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "debug.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "TextMetrics.h"

#include "support/Translator.h"

#include <sstream>


namespace lyx {

using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::endl;


namespace {

typedef Translator<std::string, InsetBox::BoxType> BoxTranslator;
typedef Translator<docstring, InsetBox::BoxType> BoxTranslatorLoc;

BoxTranslator const init_boxtranslator()
{
	BoxTranslator translator("Boxed", InsetBox::Boxed);
	translator.addPair("Frameless", InsetBox::Frameless);
	translator.addPair("ovalbox", InsetBox::ovalbox);
	translator.addPair("Ovalbox", InsetBox::Ovalbox);
	translator.addPair("Shadowbox", InsetBox::Shadowbox);
	translator.addPair("Doublebox",InsetBox::Doublebox);
	return translator;
}


BoxTranslatorLoc const init_boxtranslator_loc()
{
	BoxTranslatorLoc translator(_("Boxed"), InsetBox::Boxed);
	translator.addPair(_("Frameless"), InsetBox::Frameless);
	translator.addPair(_("ovalbox"), InsetBox::ovalbox);
	translator.addPair(_("Ovalbox"), InsetBox::Ovalbox);
	translator.addPair(_("Shadowbox"), InsetBox::Shadowbox);
	translator.addPair(_("Doublebox"), InsetBox::Doublebox);
	return translator;
}


BoxTranslator const & boxtranslator()
{
	static BoxTranslator translator = init_boxtranslator();
	return translator;
}


BoxTranslatorLoc const & boxtranslator_loc()
{
	static BoxTranslatorLoc translator = init_boxtranslator_loc();
	return translator;
}

} // anon


void InsetBox::init()
{
	setButtonLabel();
}


InsetBox::InsetBox(BufferParams const & bp, string const & label)
	: InsetCollapsable(bp), params_(label)
{
	init();
}


InsetBox::InsetBox(InsetBox const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


InsetBox::~InsetBox()
{
	InsetBoxMailer(*this).hideDialog();
}


Inset * InsetBox::clone() const
{
	return new InsetBox(*this);
}


docstring const InsetBox::editMessage() const
{
	return _("Opened Box Inset");
}


void InsetBox::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetBox::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
	setButtonLabel();
}


void InsetBox::setButtonLabel()
{
	Font font(Font::ALL_SANE);
	font.decSize();
	font.decSize();

	BoxType btype = boxtranslator().find(params_.type);

	docstring label;
	label += _("Box");
	label += " (";
	if (btype == Frameless) {
		if (params_.use_parbox)
			label += _("Parbox");
		else
			label += _("Minipage");
	} else
		label += boxtranslator_loc().find(btype);
	label += ")";

	setLabel(label);

	font.setColor(Color::foreground);
	setLabelFont(font);
}


bool InsetBox::hasFixedWidth() const
{
      return params_.inner_box || params_.special != "width";
}


bool InsetBox::metrics(MetricsInfo & m, Dimension & dim) const
{
	MetricsInfo mi = m;
	// first round in order to know the minimum size.
	InsetCollapsable::metrics(mi, dim);
	if (hasFixedWidth())
		mi.base.textwidth =	std::max(dim.width(),
			params_.width.inPixels(m.base.textwidth));
	InsetCollapsable::metrics(mi, dim);
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


bool InsetBox::forceDefaultParagraphs(idx_type) const
{
	return !params_.inner_box;
}


bool InsetBox::showInsetDialog(BufferView * bv) const
{
	InsetBoxMailer(const_cast<InsetBox &>(*this)).showDialog(bv);
	return true;
}


void InsetBox::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		//lyxerr << "InsetBox::dispatch MODIFY" << endl;
		InsetBoxMailer::string2params(to_utf8(cmd.argument()), params_);
		setButtonLabel();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetBoxMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetBoxMailer(*this).showDialog(&cur.bv());
			break;
		}
		InsetCollapsable::doDispatch(cur, cmd);
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetBox::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;
	case LFUN_BREAK_PARAGRAPH:
		if (params_.inner_box) {
			return InsetCollapsable::getStatus(cur, cmd, flag);
		} else {
			flag.enabled(false);
			return true;
		}

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}


int InsetBox::latex(Buffer const & buf, odocstream & os,
		    OutputParams const & runparams) const
{
	BoxType btype = boxtranslator().find(params_.type);

	string width_string = params_.width.asLatexString();
	bool stdwidth(false);
	if (params_.inner_box &&
			(width_string.find("1.0\\columnwidth") != string::npos
			|| width_string.find("1.0\\textwidth") != string::npos)) {
		stdwidth = true;
		switch (btype) {
		case Frameless:
			break;
		case Boxed:
			width_string += " - 2\\fboxsep - 2\\fboxrule";
			break;
		case ovalbox:
			width_string += " - 2\\fboxsep - 0.8pt";
			break;
		case Ovalbox:
			width_string += " - 2\\fboxsep - 1.6pt";
			break;
		case Shadowbox:
			// Shadow falls outside right margin... opinions?
			width_string += " - 2\\fboxsep - 2\\fboxrule"/* "-\\shadowsize"*/;
			break;
		case Doublebox:
			width_string += " - 2\\fboxsep - 7.5\\fboxrule - 1.0pt";
			break;
		}
	}

	int i = 0;
	os << "%\n";
	// Adapt to column/text width correctly also if paragraphs indented:
	if (stdwidth)
		os << "\\noindent";

	switch (btype) {
	case Frameless:
		break;
	case Boxed:
		os << "\\framebox";
		if (!params_.inner_box) {
			os << "{\\makebox";
			// Special widths, see usrguide §3.5
			// FIXME UNICODE
			if (params_.special != "none") {
				os << "[" << params_.width.value()
				   << '\\' << from_utf8(params_.special)
				   << ']';
			} else
				os << '[' << from_ascii(width_string)
				   << ']';
			if (params_.hor_pos != 'c')
				os << "[" << params_.hor_pos << "]";
		}

		os << "{";
		break;
	case ovalbox:
		os << "\\ovalbox{";
		break;
	case Ovalbox:
		os << "\\Ovalbox{";
		break;
	case Shadowbox:
		os << "\\shadowbox{";
		break;
	case Doublebox:
		os << "\\doublebox{";
		break;
	}

	if (params_.inner_box) {
		if (params_.use_parbox)
			os << "\\parbox";
		else
			os << "\\begin{minipage}";

		os << "[" << params_.pos << "]";
		if (params_.height_special == "none") {
			// FIXME UNICODE
			os << '[' << from_ascii(params_.height.asLatexString())
			   << ']';
		} else {
			// Special heights
			// FIXME UNICODE
			os << "[" << params_.height.value()
			   << '\\' << from_utf8(params_.height_special)
			   << ']';
		}
		if (params_.inner_pos != params_.pos)
			os << "[" << params_.inner_pos << "]";

		// FIXME UNICODE
		os << '{' << from_ascii(width_string) << '}';

		if (params_.use_parbox)
			os << "{";
		os << "%\n";
		i += 1;
	}

	i += InsetText::latex(buf, os, runparams);

	if (params_.inner_box) {
		if (params_.use_parbox)
			os << "%\n}";
		else
			os << "%\n\\end{minipage}";
	}

	switch (btype) {
	case Frameless:
		break;
	case Boxed:
		if (!params_.inner_box)
			os << "}"; // for makebox
		os << "}";
		break;
	case ovalbox:
	case Ovalbox:
	case Doublebox:
	case Shadowbox:
		os << "}";
		break;
	}
	os << "%\n";

	i += 3;

	return i;
}


int InsetBox::plaintext(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	BoxType const btype = boxtranslator().find(params_.type);

	switch (btype) {
		case Frameless: break;
		case Boxed:     os << "[\n";  break;
		case ovalbox:   os << "(\n";  break;
		case Ovalbox:   os << "((\n"; break;
		case Shadowbox: os << "[/\n"; break;
		case Doublebox: os << "[[\n"; break;
	}

	InsetText::plaintext(buf, os, runparams);

	int len = 0;
	switch (btype) {
		case Frameless: os << "\n";            break;
		case Boxed:     os << "\n]";  len = 1; break;
		case ovalbox:   os << "\n)";  len = 1; break;
		case Ovalbox:   os << "\n))"; len = 2; break;
		case Shadowbox: os << "\n/]"; len = 2; break;
		case Doublebox: os << "\n]]"; len = 2; break;
	}

	return PLAINTEXT_NEWLINE + len; // len chars on a separate line
}


int InsetBox::docbook(Buffer const & buf, odocstream & os,
		      OutputParams const & runparams) const
{
	return InsetText::docbook(buf, os, runparams);
}


void InsetBox::validate(LaTeXFeatures & features) const
{
	BoxType btype = boxtranslator().find(params_.type);
	switch (btype) {
	case Frameless:
		break;
	case Boxed:
		features.require("calc");
		break;
	case ovalbox:
	case Ovalbox:
	case Shadowbox:
	case Doublebox:
		features.require("calc");
		features.require("fancybox");
		break;
	}
	InsetText::validate(features);
}


InsetBoxMailer::InsetBoxMailer(InsetBox & inset)
	: inset_(inset)
{}


string const InsetBoxMailer::name_ = "box";


string const InsetBoxMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


string const InsetBoxMailer::params2string(InsetBoxParams const & params)
{
	ostringstream data;
	data << "box" << ' ';
	params.write(data);
	return data.str();
}


void InsetBoxMailer::string2params(string const & in,
				   InsetBoxParams & params)
{
	params = InsetBoxParams(string());
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetBoxMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "Box")
		return print_mailer_error("InsetBoxMailer", in, 2, "Box");

	params.read(lex);
}


InsetBoxParams::InsetBoxParams(string const & label)
	: type(label),
	  use_parbox(false),
	  inner_box(true),
	  width(Length("100col%")),
	  special("none"),
	  pos('t'),
	  hor_pos('c'),
	  inner_pos('t'),
	  height(Length("1in")),
	  height_special("totalheight") // default is 1\\totalheight
{}


void InsetBoxParams::write(ostream & os) const
{
	os << "Box " << type << "\n";
	os << "position \"" << pos << "\"\n";
	os << "hor_pos \"" << hor_pos << "\"\n";
	os << "has_inner_box " << inner_box << "\n";
	os << "inner_pos \"" << inner_pos << "\"\n";
	os << "use_parbox " << use_parbox << "\n";
	os << "width \"" << width.asString() << "\"\n";
	os << "special \"" << special << "\"\n";
	os << "height \"" << height.asString() << "\"\n";
	os << "height_special \"" << height_special << "\"\n";
}


void InsetBoxParams::read(Lexer & lex)
{
	if (!lex.isOK())
		return;

	lex.next();
	type = lex.getString();

	if (!lex)
		return;

	lex.next();
	string token;
	token = lex.getString();
	if (!lex)
		return;
	if (token == "position") {
		lex.next();
		// The [0] is needed. We need the first and only char in
		// this string -- MV
		pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'position'-tag!" << token << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "hor_pos") {
		lex.next();
		hor_pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'hor_pos'-tag!" << token << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "has_inner_box") {
		lex.next();
		inner_box = lex.getInteger();
	} else {
		lyxerr << "InsetBox::Read: Missing 'has_inner_box'-tag!" << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "inner_pos") {
		lex.next();
		inner_pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'inner_pos'-tag!"
			<< token << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "use_parbox") {
		lex.next();
		use_parbox = lex.getInteger();
	} else {
		lyxerr << "InsetBox::Read: Missing 'use_parbox'-tag!" << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "width") {
		lex.next();
		width = Length(lex.getString());
	} else {
		lyxerr << "InsetBox::Read: Missing 'width'-tag!" << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "special") {
		lex.next();
		special = lex.getString();
	} else {
		lyxerr << "InsetBox::Read: Missing 'special'-tag!" << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "height") {
		lex.next();
		height = Length(lex.getString());
	} else {
		lyxerr << "InsetBox::Read: Missing 'height'-tag!" << endl;
		lex.pushToken(token);
	}

	lex.next();
	token = lex.getString();
	if (!lex)
		return;
	if (token == "height_special") {
		lex.next();
		height_special = lex.getString();
	} else {
		lyxerr << "InsetBox::Read: Missing 'height_special'-tag!" << endl;
		lex.pushToken(token);
	}
}


} // namespace lyx
