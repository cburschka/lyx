/**
 * \file ref_inset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ref_inset.h"

#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "buffer.h"
#include "cursor.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "math_data.h"
#include "math_factory.h"
#include "math_support.h"
#include "outputparams.h"
#include "sgml.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

using std::string;
using std::auto_ptr;
using std::endl;



RefInset::RefInset()
	: CommandInset("ref")
{}


RefInset::RefInset(string const & data)
	: CommandInset(data)
{}


auto_ptr<InsetBase> RefInset::clone() const
{
	return auto_ptr<InsetBase>(new RefInset(*this));
}


void RefInset::infoize(std::ostream & os) const
{
	os << "Ref: " << cell(0);
}


void RefInset::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ref") {
			MathArray ar;
			if (createMathInset_fromDialogStr(cmd.argument, ar)) {
				*this = *ar[0].nucleus()->asRefInset();
				break;
			}
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			lyxerr << "trying to goto ref '" << asString(cell(0)) << "'" << endl;
			cur.bv().dispatch(FuncRequest(LFUN_REF_GOTO, asString(cell(0))));
			break;
		}
		if (cmd.button() == mouse_button::button1) {
			// Eventually trigger dialog with button 3, not 1
			string const data = createDialogStr("ref");
			cur.bv().owner()->getDialogs().show("ref", data, this);
			break;
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		// eat other mouse commands
		break;

	default:
		CommandInset::priv_dispatch(cur, cmd);
		break;
	}
}


string const RefInset::screenLabel() const
{
	string str;
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (commandname() == types[i].latex_name) {
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
	if (commandname() == "vref" || commandname() == "vpageref")
		features.require("varioref");
	else if (commandname() == "prettyref")
		features.require("prettyref");
}


int RefInset::plaintext(std::ostream & os, OutputParams const &) const
{
	os << '[' << asString(cell(0)) << ']';
	return 0;
}


int RefInset::linuxdoc(std::ostream & os, OutputParams const &) const
{
	os << "<ref id=\"" << asString(cell(0))
	   << "\" name=\"" << asString(cell(1)) << "\">";
	return 0;
}


int RefInset::docbook(Buffer const & buf, std::ostream & os, OutputParams const & runparams) const
{
	if (cell(1).empty()) {
		os << "<xref linkend=\"" << sgml::cleanID(buf, runparams, asString(cell(0)));
		if (runparams.flavor == OutputParams::XML) 
			os << "\"/>";
		else
			os << "\">";
	} else {
		os << "<link linkend=\"" << sgml::cleanID(buf, runparams, asString(cell(0)))
		   << "\">" << asString(cell(1)) << "</link>";
	}

	return 0;
}




RefInset::ref_type_info RefInset::types[] = {
	{ "ref",       N_("Standard"),              N_("Ref: ")},
	{ "eqref",     N_("Equation"),              N_("EqRef: ")},
	{ "pageref",   N_("Page Number"),           N_("Page: ")},
	{ "vpageref",  N_("Textual Page Number"),   N_("TextPage: ")},
	{ "vref",      N_("Standard+Textual Page"), N_("Ref+Text: ")},
	{ "prettyref", N_("PrettyRef"),             N_("PrettyRef: ")},
	{ "", "", "" }
};
