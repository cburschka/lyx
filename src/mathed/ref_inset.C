
#include "ref_inset.h"
#include "math_cursor.h"
#include "commandtags.h"
#include "formulabase.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/Dialogs.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"

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


int RefInset::dispatch(string const & cmd, idx_type, pos_type) 
{
	if (cmd == "mouse 3") {
		cerr << "trying to goto ref" << cell(0) << "\n";
		mathcursor->formula()->view()->owner()->getLyXFunc()->
			dispatch(LFUN_REF_GOTO, asString(cell(0)));
		return 1; // dispatched
	}
	
	if (cmd == "mouse 1") {
		cerr << "trying to open ref" << cell(0) << "\n";
		// Eventually trigger dialog with button 3 not 1
//		mathcursor->formula()->view()->owner()->getDialogs()
//			->showRef(this);
		return 1; // dispatched
	}

	return 0; // undispatched
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
	os << "[" << asString(cell(0)) << "]";
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

RefInset::type_info RefInset::types[] = {
	{ "ref",	N_("Standard"),			N_("Ref: ")},
	{ "pageref",	N_("Page Number"),		N_("Page: ")},
	{ "vpageref",	N_("Textual Page Number"),	N_("TextPage: ")},
	{ "vref",	N_("Standard+Textual Page"),	N_("Ref+Text: ")},
	{ "prettyref",	N_("PrettyRef"),		N_("PrettyRef: ")},
	{ "", "", "" }
};
