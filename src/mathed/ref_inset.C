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
#include "math_data.h"
#include "math_factory.h"

#include "BufferView.h"
#include "dispatchresult.h"
#include "debug.h"
#include "funcrequest.h"
#include "math_support.h"
#include "gettext.h"
#include "LaTeXFeatures.h"

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


DispatchResult
RefInset::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ref") {
			MathArray ar;
			if (!createMathInset_fromDialogStr(cmd.argument, ar))
				return DispatchResult(false);

			*this = *ar[0].nucleus()->asRefInset();

			return DispatchResult(true, true);
		}
		break;
	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			lyxerr << "trying to goto ref" << cell(0) << endl;
			cmd.view()->dispatch(FuncRequest(LFUN_REF_GOTO, asString(cell(0))));
			return DispatchResult(true, true);
		}
		if (cmd.button() == mouse_button::button1) {
			// Eventually trigger dialog with button 3
			// not 1
			string const data = createDialogStr("ref");
			cmd.view()->owner()->getDialogs().
				show("ref", data, this);
			return DispatchResult(true, true);
		}
		break;
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		// eat other mouse commands
		return DispatchResult(true, true);
	default:
		return CommandInset::priv_dispatch(cmd, idx, pos);
	}
	// not our business
	return DispatchResult(false);
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
	   << "\" name=\"" << asString(cell(1)) << "\" >";
	return 0;
}


int RefInset::docbook(std::ostream & os, OutputParams const &) const
{
	if (cell(1).empty()) {
		os << "<xref linkend=\"" << asString(cell(0)) << "\">";
	} else {
		os << "<link linkend=\"" << asString(cell(0))
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
