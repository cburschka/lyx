
#include <config.h>

#include "ref_inset.h"
#include "funcrequest.h"
#include "formulabase.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/Dialogs.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "Lsstream.h"
#include "math_parser.h"
#include "support/lstrings.h"


RefInset::RefInset()
	: CommandInset("ref")
{}


RefInset::RefInset(string const & data)
	: CommandInset(data)
{}


MathInset * RefInset::clone() const
{
	return new RefInset(*this);
}


void RefInset::infoize(std::ostream & os) const
{
	os << "Ref: " << cell(0);
}


dispatch_result
RefInset::dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
		case LFUN_MOUSE_RELEASE:
			if (cmd.button() == mouse_button::button3) {
				lyxerr << "trying to goto ref" << cell(0) << "\n";
				cmd.view()->dispatch(FuncRequest(LFUN_REF_GOTO, asString(cell(0))));
				return DISPATCHED;
			}
			if (cmd.button() == mouse_button::button1) {
				// Eventually trigger dialog with button 3
				// not 1
				ostringstream data;
				data << "ref LatexCommand ";
				WriteStream wsdata(data);
				write(wsdata);
				wsdata << "\n\\end_inset\n\n";

				cmd.view()->owner()->getDialogs().
					show("ref", data.str(), this);
				return DISPATCHED;
			}
			break;
		case LFUN_MOUSE_PRESS:
		case LFUN_MOUSE_MOTION:
			// eat other mouse commands
			return DISPATCHED;
		default:
			return CommandInset::dispatch(cmd, idx, pos);
	}
	// not our business
	return UNDISPATCHED;
}


string RefInset::screenLabel() const
{
	string str;
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (name_ == types[i].latex_name) {
			str = _(types[i].short_gui_name);
			break;
		}
	str += asString(cell(0));

	//if (/* !isLatex && */ !cell(0).empty()) {
	//	str += "||";
	//	str += asString(cell(1));
	//}
	return str;
}


void RefInset::validate(LaTeXFeatures & features) const
{
	if (name_ == "vref" || name_ == "vpageref")
		features.require("varioref");
	else if (name_ == "prettyref")
		features.require("prettyref");
}


int RefInset::ascii(std::ostream & os, int) const
{
	os << '[' << asString(cell(0)) << ']';
	return 0;
}


int RefInset::linuxdoc(std::ostream & os) const
{
	os << "<ref id=\"" << asString(cell(0))
	   << "\" name=\"" << asString(cell(1)) << "\" >";
	return 0;
}


int RefInset::docbook(std::ostream & os, bool) const
{
	if (cell(1).empty()) {
		os << "<xref linkend=\"" << asString(cell(0)) << "\">";
	} else {
		os << "<link linkend=\"" << asString(cell(0))
		   << "\">" << asString(cell(1)) << "</link>";
	}

	return 0;
}


dispatch_result RefInset::localDispatch(FuncRequest const & cmd)
{
	if (cmd.action != LFUN_INSET_MODIFY || cmd.getArg(0) != "ref")
		return UNDISPATCHED;

	MathArray ar;
	if (!string2RefInset(cmd.argument, ar))
		return UNDISPATCHED;

	*this = *ar[0].nucleus()->asRefInset();
// 	if (cmd.view())
//                 // This does not compile because updateInset expects
//                 // an Inset* and 'this' isn't.
// 		cmd.view()->updateInset(this, true);
	return DISPATCHED;
}


bool string2RefInset(string const & str, MathArray & ar)
{
	string name;
	string body = split(str, name, ' ');

	if (name != "ref")
		return false;

	// body comes with a head "LatexCommand " and a
	// tail "\nend_inset\n\n". Strip them off.
	string trimmed;
	body = split(body, trimmed, ' ');
	split(body, trimmed, '\n');

	mathed_parse_cell(ar, trimmed);
	if (ar.size() != 1)
		return false;

	return ar[0].nucleus()->asRefInset();
}


RefInset::ref_type_info RefInset::types[] = {
	{ "ref",	N_("Standard"),			N_("Ref: ")},
	{ "pageref",	N_("Page Number"),		N_("Page: ")},
	{ "vpageref",	N_("Textual Page Number"),	N_("TextPage: ")},
	{ "vref",	N_("Standard+Textual Page"),	N_("Ref+Text: ")},
	{ "prettyref",	N_("PrettyRef"),		N_("PrettyRef: ")},
	{ "", "", "" }
};
