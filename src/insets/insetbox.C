/**
 * \file insetbox.C
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

#include "insetbox.h"

#include "BufferView.h"
#include "dispatchresult.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "support/std_sstream.h"

using std::auto_ptr;
using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::endl;


namespace {

BoxTranslator const init_boxtranslator() {
	BoxTranslator translator("Boxed", InsetBox::Boxed);
	translator.addPair("Frameless", InsetBox::Frameless);
	translator.addPair("ovalbox", InsetBox::ovalbox);
	translator.addPair("Ovalbox", InsetBox::Ovalbox);
	translator.addPair("Shadowbox", InsetBox::Shadowbox);
	translator.addPair("Doublebox",InsetBox::Doublebox);
	return translator;
}


BoxTranslator const init_boxtranslator_loc() {
	BoxTranslator translator(_("Boxed"), InsetBox::Boxed);
	translator.addPair(_("Frameless"), InsetBox::Frameless);
	translator.addPair(_("ovalbox"), InsetBox::ovalbox);
	translator.addPair(_("Ovalbox"), InsetBox::Ovalbox);
	translator.addPair(_("Shadowbox"), InsetBox::Shadowbox);
	translator.addPair(_("Doublebox"), InsetBox::Doublebox);
	return translator;
}


BoxTranslator const & boxtranslator() {
	static BoxTranslator translator = init_boxtranslator();
	return translator;
}


BoxTranslator const & boxtranslator_loc() {
	static BoxTranslator translator = init_boxtranslator_loc();
	return translator;
}

} // anon


void InsetBox::init()
{
	setInsetName("Box");
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
	InsetBoxMailer mailer(*this);
	mailer.hideDialog();
}


auto_ptr<InsetBase> InsetBox::clone() const
{
	return auto_ptr<InsetBase>(new InsetBox(*this));
}


string const InsetBox::editMessage() const
{
	return _("Opened Box Inset");
}


void InsetBox::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetBox::read(Buffer const & buf, LyXLex & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
	setButtonLabel();
}


void InsetBox::setButtonLabel()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();

	BoxType btype = boxtranslator().find(params_.type);
	if (btype == Frameless) {
		if (params_.use_parbox)
			setLabel(_("Parbox"));
		else
			setLabel(_("Minipage"));
	} else
		setLabel(boxtranslator_loc().find(btype));

	font.setColor(LColor::foreground);
	setBackgroundColor(LColor::background);
	setLabelFont(font);
}


void InsetBox::metrics(MetricsInfo & m, Dimension & dim) const
{
	MetricsInfo mi = m;
	mi.base.textwidth = params_.width.inPixels(m.base.textwidth); 
	InsetCollapsable::metrics(mi, dim);
	//if (params_.inner_box && isOpen())
	//	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


bool InsetBox::showInsetDialog(BufferView * bv) const
{
	InsetBoxMailer(const_cast<InsetBox &>(*this)).showDialog(bv);
	return true;
}


DispatchResult
InsetBox::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	DispatchResult result(false);
	BufferView * bv = cmd.view();

	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		lyxerr << "InsetBox::dispatch MODIFY" << endl;
		InsetBoxMailer::string2params(cmd.argument, params_);
		setButtonLabel();
		bv->updateInset(this);
		result.dispatched(true);
		result.update(true);
		return result;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetBoxMailer(*this).updateDialog(bv);
		result.dispatched(true);
		return result;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetBoxMailer(*this).showDialog(bv);
			return DispatchResult(true);
		}
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);

	default:
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


int InsetBox::latex(Buffer const & buf, ostream & os,
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
			if (params_.special != "none") {
				os << "[" << params_.width.value()
				   << "\\" << params_.special << "]";
			} else
				os << "[" << width_string << "]";
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
			os << "[" << params_.height.asLatexString() << "]";
		} else {
			// Special heights
			os << "[" << params_.height.value()
			   << "\\" << params_.height_special << "]";
		}
		if (params_.inner_pos != params_.pos)
			os << "[" << params_.inner_pos << "]";

		os << "{" << width_string << "}";

		if (params_.use_parbox)
			os << "{";
		os << "%\n";
		i += 1;
	}

	i += inset.latex(buf, os, runparams);

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


int InsetBox::linuxdoc(Buffer const & buf, std::ostream & os,
		       OutputParams const & runparams) const
{
	return inset.linuxdoc(buf, os, runparams);
}


int InsetBox::docbook(Buffer const & buf, std::ostream & os,
		      OutputParams const & runparams) const
{
	return inset.docbook(buf, os, runparams);
}


int InsetBox::plaintext(Buffer const & buf, std::ostream & os,
		    OutputParams const & runparams) const
{
	BoxType const btype = boxtranslator().find(params_.type);

	switch (btype) {
		case Frameless: break;
		case Boxed:     os << "[";  break;
		case ovalbox:   os << "(";  break;
		case Ovalbox:   os << "(("; break;
		case Shadowbox: os << "[";  break;
		case Doublebox: os << "[["; break;
	}

	int i = inset.plaintext(buf, os, runparams);

	switch (btype) {
		case Frameless: break;
		case Boxed:     os << "]";  break;
		case ovalbox:   os << ")";  break;
		case Ovalbox:   os << "))"; break;
		case Shadowbox: os << "]/"; break;
		case Doublebox: os << "]]"; break;
	}

	return i;
}


void InsetBox::validate(LaTeXFeatures & features) const
{
	features.require("calc");
	BoxType btype = boxtranslator().find(params_.type);
	switch (btype) {
	case Frameless:
	case Boxed:
		break;
	case ovalbox:
	case Ovalbox:
	case Shadowbox:
	case Doublebox:
		features.require("fancybox");
		break;
	}
	inset.validate(features);
}


InsetBoxMailer::InsetBoxMailer(InsetBox & inset)
	: inset_(inset)
{
}


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
	LyXLex lex(0,0);
	lex.setStream(data);

	string token;
	lex.next();
	token = lex.getString();
	lex.next();

	params.read(lex);
}


InsetBoxParams::InsetBoxParams(string const & label)
	: type(label),
	  use_parbox(false),
	  inner_box(true),
	  width(LyXLength("100col%")),
	  special("none"),
	  pos('t'),
	  hor_pos('c'),
	  inner_pos('t'),
	  height(LyXLength("1in")),
	  height_special("totalheight") // default is 1\\totalheight
{}


void InsetBoxParams::write(ostream & os) const
{
	os << type << "\n";
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


void InsetBoxParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		type = lex.getString();
	}
	string token;
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "position") {
		lex.next();
		// The [0] is needed. We need the first and only char in
		// this string -- MV
		pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'position'-tag!" << token << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "hor_pos") {
		lex.next();
		hor_pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'hor_pos'-tag!" << token << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "has_inner_box") {
		lex.next();
		inner_box = lex.getInteger();
	} else {
		lyxerr << "InsetBox::Read: Missing 'has_inner_box'-tag!" << endl;
		lex.pushToken(token);
	}

	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "inner_pos") {
		lex.next();
		inner_pos = lex.getString()[0];
	} else {
		lyxerr << "InsetBox::Read: Missing 'inner_pos'-tag!"
			<< token << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "use_parbox") {
		lex.next();
		use_parbox = lex.getInteger();
	} else {
		lyxerr << "InsetBox::Read: Missing 'use_parbox'-tag!" << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "width") {
		lex.next();
		width = LyXLength(lex.getString());
	} else {
		lyxerr << "InsetBox::Read: Missing 'width'-tag!" << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "special") {
		lex.next();
		special = lex.getString();
	} else {
		lyxerr << "InsetBox::Read: Missing 'special'-tag!" << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "height") {
		lex.next();
		height = LyXLength(lex.getString());
	} else {
		lyxerr << "InsetBox::Read: Missing 'height'-tag!" << endl;
		lex.pushToken(token);
	}
	if (!lex.isOK())
		return;
	lex.next();
	token = lex.getString();
	if (token == "height_special") {
		lex.next();
		height_special = lex.getString();
	} else {
		lyxerr << "InsetBox::Read: Missing 'height_special'-tag!" << endl;
		lex.pushToken(token);
	}
}
